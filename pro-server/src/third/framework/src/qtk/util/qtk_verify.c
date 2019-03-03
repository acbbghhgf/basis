#include "qtk_verify.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cpuid.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include "wtk/core/wtk_str.h"
#include "wtk/os/wtk_socket.h"
#include "qtk/http/qtk_request.h"
#include "qtk/http/qtk_response.h"
#include "qtk/http/qtk_http_parser.h"
#include "qtk/core/cJSON.h"
#include "qtk/util/qtk_rsa.h"

#include <sys/types.h>
#include <ifaddrs.h>


static char pub_key[] = "-----BEGIN RSA PUBLIC KEY-----\n\
MIGJAoGBALduScYh/dyHQm5uwZd0h/Wn9iqp7iczKehh2y93oWY8GHVdoFKxFddm\n\
reMrJKZmPzXAN9UfxLbOZagDfa0yZAycUBy6l5v/Avm2b+S0zIm+14T2MGg5MFTv\n\
A8DuIGO/lYyJOgD/vze03YGo3bXXVD+mmZuuXn8LOdlhNnntXV8pAgMBAAE=\n\
-----END RSA PUBLIC KEY-----\n\
";
static const char phrase[] = "qdreamer";


#ifdef BIGENDIAN
typedef struct {
    unsigned int h;
    unsigned int l;
} u64;
#define DWORD(x) (unsigned int)((x)[3] + ((x)[2] << 8) + ((x)[1] << 16) + ((x)[0] << 24))
#define WORD(x) (unsigned short)((x)[1] + ((x)[0] << 8))
#define QWORD(x) _U64(DWORD(x+4), DWORD(x))
#else
typedef struct {
    unsigned int l;
    unsigned int h;
} u64;
#define DWORD(x) (unsigned int)((x)[0] + ((x)[1] << 8) + ((x)[2] << 16) + ((x)[3] << 24))
#define WORD(x) (unsigned short)((x)[0] + ((x)[1] << 8))
#define QWORD(x) _U64(DWORD(x), DWORD(x+4))
#endif

#define FLAG_NO_FILE_OFFSET (1<<0)
#define FLAG_STOP_AT_EOT (1<<1)
#define SUPPORTED_SMBIOS_VER 0x030101

#define EFI_NOT_FOUND (-1)
#define EFI_NO_SMBIOS (-2)

inline static u64 _U64(unsigned int low, unsigned int high) {
    u64 a = {.l=low, .h=high};
    return a;
}

typedef struct dmi_header dmi_header_t;

struct dmi_header {
    unsigned char type;
    unsigned char length;
    unsigned short handle;
    unsigned char *data;
};


static wtk_string_t *cpu_id = NULL;
static wtk_string_t *mac_addr = NULL;
static wtk_string_t *base_board_seq = NULL;


static int address_from_efi(off_t *address)
{
#if defined(__linux__)
    FILE *efi_systab;
    const char *filename;
    char linebuf[64];
#elif defined(__FreeBSD__)
    char addrstr[KENV_MVALLEN + 1];
#endif
    int ret;

    *address = 0; /* Prevent compiler warning */

#if defined(__linux__)
    /*
     * Linux up to 2.6.6: /proc/efi/systab
     * Linux 2.6.7 and up: /sys/firmware/efi/systab
     */
    if ((efi_systab = fopen(filename = "/sys/firmware/efi/systab", "r")) == NULL
        && (efi_systab = fopen(filename = "/proc/efi/systab", "r")) == NULL)
    {
        /* No EFI interface, fallback to memory scan */
        return EFI_NOT_FOUND;
    }
    ret = EFI_NO_SMBIOS;
    while ((fgets(linebuf, sizeof(linebuf) - 1, efi_systab)) != NULL)
    {
        char *addrp = strchr(linebuf, '=');
        *(addrp++) = '\0';
        if (strcmp(linebuf, "SMBIOS3") == 0
            || strcmp(linebuf, "SMBIOS") == 0)
        {
            *address = strtoull(addrp, NULL, 0);
            ret = 0;
            break;
        }
    }
    if (fclose(efi_systab) != 0)
        perror(filename);

    if (ret == EFI_NO_SMBIOS)
        fprintf(stderr, "%s: SMBIOS entry point missing\n", filename);
#elif defined(__FreeBSD__)
    /*
     * On FreeBSD, SMBIOS anchor base address in UEFI mode is exposed
     * via kernel environment:
     * https://svnweb.freebsd.org/base?view=revision&revision=307326
     */
    ret = kenv(KENV_GET, "hint.smbios.0.mem", addrstr, sizeof(addrstr));
    if (ret == -1)
    {
        if (errno != ENOENT)
            perror("kenv");
        return EFI_NOT_FOUND;
    }

    *address = strtoull(addrstr, NULL, 0);
    ret = 0;
#else
    ret = EFI_NOT_FOUND;
#endif
    return ret;
}


static int myread(int fd, unsigned char *buf, size_t count, const char *prefix)
{
    ssize_t r = 1;
    size_t r2 = 0;

    while (r2 != count && r != 0)
    {
        r = read(fd, buf + r2, count - r2);
        if (r == -1)
        {
            if (errno != EINTR)
            {
                close(fd);
                perror(prefix);
                return -1;
            }
        }
        else
            r2 += r;
    }

    if (r2 != count)
    {
        close(fd);
        fprintf(stderr, "%s: Unexpected end of file\n", prefix);
        return -1;
    }

    return 0;
}


static void* mem_chunk(off_t base, size_t len, const char *devmem) {
    void *p;
    int fd;
#ifdef USE_MMAP
    struct stat statbuf;
    off_t mmoffset;
    void *mmp;
#endif

    if ((fd = open(devmem, O_RDONLY)) == -1)
    {
        perror(devmem);
        return NULL;
    }

    if ((p = malloc(len)) == NULL)
    {
        perror("malloc");
        goto out;
    }

#ifdef USE_MMAP
    if (fstat(fd, &statbuf) == -1)
    {
        fprintf(stderr, "%s: ", devmem);
        perror("stat");
        goto err_free;
    }

    /*
     * mmap() will fail with SIGBUS if trying to map beyond the end of
     * the file.
     */
    if (S_ISREG(statbuf.st_mode) && base + (off_t)len > statbuf.st_size)
    {
        fprintf(stderr, "mmap: Can't map beyond end of file %s\n",
                devmem);
        goto err_free;
    }

#ifdef _SC_PAGESIZE
    mmoffset = base % sysconf(_SC_PAGESIZE);
#else
    mmoffset = base % getpagesize();
#endif /* _SC_PAGESIZE */
    /*
     * Please note that we don't use mmap() for performance reasons here,
     * but to workaround problems many people encountered when trying
     * to read from /dev/mem using regular read() calls.
     */
    mmp = mmap(NULL, mmoffset + len, PROT_READ, MAP_SHARED, fd, base - mmoffset);
    if (mmp == MAP_FAILED)
        goto try_read;

    memcpy(p, (u8 *)mmp + mmoffset, len);

    if (munmap(mmp, mmoffset + len) == -1)
    {
        fprintf(stderr, "%s: ", devmem);
        perror("munmap");
    }

    goto out;

try_read:
#endif /* USE_MMAP */
    if (lseek(fd, base, SEEK_SET) == -1)
    {
        fprintf(stderr, "%s: ", devmem);
        perror("lseek");
        goto err_free;
    }

    if (myread(fd, p, len, devmem) == 0)
        goto out;

err_free:
    free(p);
    p = NULL;

out:
    if (close(fd) == -1)
        perror(devmem);

    return p;
}


static int checksum(const unsigned char *buf, size_t len) {
    unsigned char sum = 0;
    size_t a;

    for (a = 0; a < len; a++) {
        sum += buf[a];
    }

    return sum == 0;
}


static void to_dmi_header(struct dmi_header *h, unsigned char *data) {
    h->type = data[0];
    h->length = data[1];
    h->handle = WORD(data+2);
    h->data = data;
}


static char* dmi_string(const struct dmi_header *dm, unsigned char s) {
    char *bp = (char *)dm->data;
    size_t i, len;

    if (s == 0)
        return "Not Specified";

    bp += dm->length;
    while (s > 1 && *bp) {
        bp += strlen(bp);
        bp++;
        s--;
    }

    if (!*bp) {
        return "<BAD INDEX>";
    }

    len = strlen(bp);
    for (i = 0; i < len; i++) {
        if (bp[i] < 32 || bp[i] == 127) {
            bp[i] = '.';
        }
    }

    return bp;
}


static void dmi_decode_cpu_id(const dmi_header_t *h, unsigned short ver) {
    const unsigned char *p = h->data + 0x08;
    char buf[32];
    int n;

    if (cpu_id) {
        wtk_string_delete(cpu_id);
    }
    n = snprintf(buf, sizeof(buf), "%02X%02X%02X%02X%02X%02X%02X%02X",
             p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
    cpu_id = wtk_string_dup_data_pad0(buf, n);
}


static void dmi_decode_base_board(const dmi_header_t *h, unsigned short ver) {
    const unsigned char *data = h->data;

    const char *p = dmi_string(h, data[0x07]);

    base_board_seq = wtk_string_dup_data_pad0((char*)p, strlen(p));
}


static void dmi_table_decode(unsigned char *buf, unsigned int len, unsigned short num,
                             unsigned short ver, unsigned int flags) {
    unsigned char *data;
    int i = 0;

    data = buf;
    while ((i < num || !num) && data + 4 <= buf + len) {
        unsigned char *next;
        struct dmi_header h;

        to_dmi_header(&h, data);

        if (h.length < 4) {
            break;
        }

        next = data + h.length;
        while ((unsigned long)(next - buf + 1) < len
               &&(next[0] != 0 || next[1] != 0))
            next++;
        next += 2;
        if ((unsigned long)(next-buf) <= len) {
            switch (h.type) {
            case 2:
                dmi_decode_base_board(&h, ver);
                break;
            case 4:
                dmi_decode_cpu_id(&h, ver);
                break;
            }
        }

        data = next;
        i++;
        if (h.type == 127 && (flags & FLAG_STOP_AT_EOT)) break;
    }
}


static void *read_file(off_t base, size_t *max_len, const char *filename) {
    int fd;
    size_t r2 = 0;
    ssize_t r;
    char *p;

    if ((fd = open(filename, O_RDONLY)) == -1) {
        if (errno != ENOENT)
            perror(filename);
        return NULL;
    }

    if (lseek(fd, base, SEEK_SET) == -1) {
        fprintf(stderr, "%s: ", filename);
        perror("lseek");
        p = NULL;
        goto out;
    }

    if ((p = malloc(*max_len)) == NULL) {
        perror("malloc");
        goto out;
    }

    do {
        r = read(fd, p + r2, *max_len - r2);
        if (r == -1) {
            if (errno != EINTR) {
                perror(filename);
                free(p);
                p = NULL;
                goto out;
            }
        }
        else
            r2 += r;
    } while (r != 0);
    *max_len = r2;
out:
    close(fd);
    return p;
}


static void dmi_table(off_t base, unsigned int len, unsigned short num, unsigned int ver,
                      const char *devmem, unsigned int flags) {
    unsigned char *buf;
    if (ver > SUPPORTED_SMBIOS_VER) {
        wtk_debug("SMBIOS implementations newer than version %u.%u.%u are not "
                  "fully supported by this version of dmidecode.\n",
                  SUPPORTED_SMBIOS_VER >> 16,
                  (SUPPORTED_SMBIOS_VER >> 8) & 0xFF,
                  SUPPORTED_SMBIOS_VER & 0xFF);
    }
    if (flags & FLAG_NO_FILE_OFFSET) {
        size_t size = len;
        buf = read_file(0, &size, devmem);
        if (num && size != len) {
            wtk_debug("Wrong DMI structures length: %u bytes announced, only %lu bytes available.\n",
                      len, (unsigned long)size);
        }
        len = size;
    } else {
        buf = mem_chunk(base, len, devmem);
    }
    dmi_table_decode(buf, len, num, ver >> 8, flags);
}


static int smbios_decode(unsigned char *buf, const char *devmem, unsigned int flags) {
    unsigned short ver;

    if (!checksum(buf, buf[0x05])
        || memcmp(buf + 0x10, "_DMI_", 5) != 0
        || !checksum(buf + 0x10, 0x0F))
        return 0;

    ver = (buf[0x06] << 8) + buf[0x07];

    switch (ver) {
    case 0x021F:
    case 0x0221:
        ver = 0x0203;
        break;
    case 0x0233:
        ver = 0x0206;
        break;
    }
    dmi_table(DWORD(buf+0x18), WORD(buf+0x16), WORD(buf+0x1c), ver << 8, devmem, flags);
    return 1;
}


static int smbios3_decode(unsigned char *buf, const char *devmem, unsigned int flags) {
    unsigned short ver;
    u64 offset;

    if (!checksum(buf, buf[0x05]))
        return 0;

    ver = (buf[0x07] << 16) + (buf[0x08] << 8) + buf[0x09];
    offset = QWORD(buf + 0x10);
    if (!(flags && FLAG_NO_FILE_OFFSET) && offset.h && sizeof(off_t) < 8) {
        wtk_debug("64-bit addresses not supported, sorry.\n");
        return 0;
    }

    dmi_table(((off_t)offset.h << 32) | offset.l,
              DWORD(buf + 0x0C), 0, ver, devmem, flags | FLAG_STOP_AT_EOT);

    return 1;
}


static int legacy_decode(unsigned char *buf, const char *devmem, unsigned int flags) {
    if (!checksum(buf, 0x0F))
        return 0;
    dmi_table(DWORD(buf + 0x08), WORD(buf + 0x06), WORD(buf + 0x0C),
              ((buf[0x0E] & 0xF0) << 12) + ((buf[0x0E] & 0x0F) << 8),
              devmem, flags);
    return 1;
}


static int get_cpuid () {
    unsigned int eax, ebx, ecx, edx;
    char id[32];
    int n;

    if (0 == __get_cpuid(1, &eax, &ebx, &ecx, &edx)) {
        return -1;
    }

    n = snprintf(id, sizeof(id), "%08X%08X", htonl(eax), htonl(edx));
    if (cpu_id) {
        wtk_string_delete(cpu_id);
    }
    cpu_id = wtk_string_dup_data_pad0(id, n);
    return 0;
}


static int get_macaddr(char *eth_name) {
    int sock;
    struct sockaddr_in sin;
    struct sockaddr sa;
    struct ifreq ifr;
    unsigned char mac[6];
    char macstr[32];
    int n;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        perror("socket");
        return -1;
    }

    strncpy(ifr.ifr_name, eth_name, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;

    memset(mac, 0, sizeof(mac));
    if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0) {
        return -1;
    }

    memcpy(&sa, &ifr.ifr_addr, sizeof(sin));
    memcpy(mac, sa.sa_data, sizeof(mac));

    int i;
    int mac_size = sizeof(mac)/sizeof mac[0];
    for(i=0; i<mac_size && mac[i]==0; i++);
    if (i==mac_size) return -1;

    n = snprintf(macstr, sizeof(macstr), "%.2X:%.2X:%.2X:%.2X:%.2X:%.2X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    mac_addr = wtk_string_dup_data_pad0(macstr, n);

    return 0;
}


typedef struct {
    qtk_response_t *resp;
    unsigned ready:1;
} recver;


static int parser_claim(void *data, qtk_http_parser_t *p) {
    recver *rv = (recver*)data;
    if (rv->resp) {
        qtk_response_delete(rv->resp);
        rv->resp = NULL;
    }
    rv->resp = qtk_response_new();
    p->pack = rv->resp;
    rv->ready = 0;
    return 0;
}


static int parser_ready(void *data, qtk_http_parser_t *p) {
    recver *rv = (recver*)data;
    rv->ready = 1;
    return 0;
}


static int verify(RSA *rsa, int sid, wtk_strbuf_t *buf) {
    cJSON *jp = NULL, *obj = NULL;
    int code = 0;
    const char *mac = "", *disk = "", *cpu = "", *srv_type = "";
    const char *sign = NULL;
    int64_t timestamp = 0;
    int ret = 1;
    char tmpbuf[1024];
    char unsigned sign2[512];
    int n;

    wtk_strbuf_push_c(buf, '\0');
    jp = cJSON_Parse(buf->data);
    obj = cJSON_GetObjectItem(jp, "code");
    if (obj && obj->type == cJSON_Number) {
        code = obj->valueint;
        if (code) goto end;
    } else {
        goto end;
    }
    obj = cJSON_GetObjectItem(jp, "sid");
    if (obj && obj->type == cJSON_String) {
        if (sid != atoi(obj->valuestring)) {
            goto end;
        }
    } else {
        goto end;
    }
    obj = cJSON_GetObjectItem(jp, "time");
    if (obj && obj->type == cJSON_Number) {
        timestamp = (uint64_t)obj->valuedouble;
    } else {
        goto end;
    }
    obj = cJSON_GetObjectItem(jp, "sign");
    if (obj && obj->type == cJSON_String) {
        sign = obj->valuestring;
    } else {
        goto end;
    }
    obj = cJSON_GetObjectItem(jp, "cpu");
    if (obj && obj->type == cJSON_String) {
        cpu = obj->valuestring;
    }
    obj = cJSON_GetObjectItem(jp, "mac");
    if (obj && obj->type == cJSON_String) {
        mac = obj->valuestring;
    }
    obj = cJSON_GetObjectItem(jp, "disk");
    if (obj && obj->type == cJSON_String) {
        disk = obj->valuestring;
    }
    obj = cJSON_GetObjectItem(jp, "service_type");
    if (obj && obj->type == cJSON_String) {
        srv_type = obj->valuestring;
    }
    snprintf(tmpbuf, sizeof(tmpbuf), "%d%s%s%s%s%d%lu", code, cpu, disk, mac, srv_type, sid, timestamp);
    n = qtk_base64_decode(sign, sign2);
    ret = !qtk_rsa_verify(tmpbuf, sign2, n, rsa);
end:
    if (jp) {
        cJSON_Delete(jp);
    }
    return ret;
}


static int request(const char *ip, int port, const char *host,
                   const char *url, const char *srv_type) {
    int ret = 0;
    int sock;
    qtk_http_parser_t *parser = NULL;
    qtk_request_t * req = NULL;
    char buf[4096];
    int n;
    wtk_string_t *v;
    recver rv = {.resp = NULL, .ready = 0};
    unsigned char *ptr_en = NULL;
    int enc_len = 0;
    struct timeval tv;
    int sid = 0;
    RSA *rsa = NULL;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_FD) {
        ret = -1;
        goto end;
    }
    wtk_debug("connecting to %s:%d\r\n", ip, port);
    ret = wtk_socket_connect2((char*)ip, port, &sock);
    if (ret) {
        goto end;
    }
    wtk_debug("connected to %s:%d\r\n", ip, port);

    /*
      send http reqest
     */
    req = qtk_request_new();
    req->method = HTTP_POST;
    wtk_string_set(&req->url, (char*)url, strlen(url));
    v = wtk_heap_dup_string(req->heap, (char*)host, strlen(host));
    qtk_request_add_hdr_s(req, "Host", v);
    v = wtk_heap_dup_string_s(req->heap, "application/x-www-form-urlencoded");
    qtk_request_add_hdr_s(req, "Content-Type", v);
    gettimeofday(&tv, 0);
    srand(tv.tv_usec);
    sid = rand();
    snprintf(buf, sizeof(buf), "%s%s%s%s%d%lu", cpu_id?cpu_id->data:"",
             base_board_seq?base_board_seq->data:"", mac_addr?mac_addr->data:"",
             srv_type, sid, tv.tv_sec*1000);
    rsa = qtk_rsa_new(pub_key, phrase);
    ptr_en = qtk_rsa_encrypt(buf, &enc_len, rsa);
    n = snprintf(buf, sizeof(buf), "cpu=%s&disk=%s&mac=%s&service_type=%s&sid=%d&time=%lu&sign=",
                 cpu_id?cpu_id->data:"", base_board_seq?base_board_seq->data:"",
                 mac_addr?mac_addr->data:"", srv_type, sid, tv.tv_sec*1000);
    qtk_base64_encode2(ptr_en, buf+n, enc_len, 1);
    qtk_request_set_body(req, buf, strlen(buf));
    qtk_request_send2(req, sock);

    /*
      parse http response
     */
    parser = qtk_http_parser_new();
    qtk_http_parser_set_handlers(parser, HTTP_RESPONSE_PARSER,
                                 parser_claim, parser_ready, NULL, &rv);
    ret = -1;
    while ((n = recv(sock, buf, sizeof(buf), 0)) > 0 || errno == EINTR) {
        parser->handler(parser, NULL, buf, n);
        if (rv.ready) {
            ret = verify(rsa, sid, rv.resp->body);
            break;
        }
    }
end:
    if (rsa) {
        qtk_rsa_free(rsa);
    }
    if (ptr_en) {
        wtk_free(ptr_en);
    }
    if (parser) {
        qtk_http_parser_delete(parser);
    }
    if (req) {
        qtk_request_delete(req);
    }
    if (rv.resp) {
        qtk_response_delete(rv.resp);
    }
    if (sock != INVALID_FD) {
        close(sock);
        sock = INVALID_FD;
    }
    return ret;
}


int qtk_hw_verify(const char *ip, int port, const char *host,
                  const char *url, const char *srv_type) {
    int ret = -1;
    int efi;
    off_t fp;
    size_t size = 0x20;
    unsigned char *buf;
    if (0) {
        buf = read_file(0, &size, "/sys/firmware/dmi/tables/smbios_entry_point");
        if (buf) {
            if (size >= 24 && memcmp(buf, "_SM3_", 5) == 0) {
                if (smbios3_decode(buf, "/sys/firmware/dmi/tables/DMI", FLAG_NO_FILE_OFFSET)) goto found;
            } else if (size >= 31 && memcmp(buf, "_SM_", 4) == 0) {
                if (smbios_decode(buf, "/sys/firmware/dmi/tables/DMI", FLAG_NO_FILE_OFFSET)) goto found;
            } else if (size >= 15 && memcmp(buf, "_DMI_", 5) == 0) {
                if (legacy_decode(buf, "/sys/firmware/dmi/tables/DMI", FLAG_NO_FILE_OFFSET)) goto found;
            }
        }
        efi = address_from_efi(&fp);
        switch (efi) {
        case EFI_NOT_FOUND:
            wtk_debug("EFI_NOT_FOUND\n");
            goto found;
        case EFI_NO_SMBIOS:
            wtk_debug("EFI_NO_SMBIOS\n");
            goto found;
        }
        if ((buf = mem_chunk(fp, 0x20, "/dev/mem")) == NULL) {
            goto end;
        }
        if (memcmp(buf, "_SM3_", 5) == 0) {
            if (smbios3_decode(buf, "/sys/firmware/dmi/tables/DMI", 0)) goto found;
        }
        if (memcmp(buf, "_SM_", 4) == 0) {
            if (smbios_decode(buf, "/sys/firmware/dmi/tables/DMI", 0)) goto found;
        }
    }
found:
    get_cpuid();
    struct ifaddrs *ifaddr, *ifa;
    if (getifaddrs(&ifaddr) == 0) {
        for (ifa = ifaddr; ifa != NULL && get_macaddr(ifa->ifa_name); ifa = ifa->ifa_next);
        freeifaddrs(ifaddr);
    }
    ret = request(ip, port, host, url, srv_type);
end:
    if (mac_addr) {
        wtk_string_delete(mac_addr);
        mac_addr = NULL;
    }
    if (cpu_id) {
        wtk_string_delete(cpu_id);
        cpu_id = NULL;
    }
    if (base_board_seq) {
        wtk_string_delete(base_board_seq);
        base_board_seq = NULL;
    }
    return ret;
}
