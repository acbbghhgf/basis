# coding: utf-8
from Crypto.Hash import SHA
from Crypto.Cipher import PKCS1_v1_5 as Cipher_pkcs1_v1_5
from Crypto.Signature import PKCS1_v1_5 as Signature_pkcs1_v1_5
from Crypto.PublicKey import RSA
import base64
import six


__all__ = ['sign', 'verify', 'enctypt', 'decrypt']

def sign(data, key, phrase=None):
    """sin

    @param data: data to be signed
    @param key: private key
    @param phrase: phrase
    @return: a hex string which is signature
    """
    pri_key = RSA.importKey(key, phrase)
    signer = Signature_pkcs1_v1_5.new(pri_key)
    digest = SHA.new()
    digest.update(data)
    signature = signer.sign(digest)
    return base64.b64encode(signature)


def verify(data, signature, key, phrase=None):
    """verify

    @param data: data content to be verified
    @param signature: a string which is the signature
    @param key: public key
    @param phrase: phrase
    @return: True or False, depending on whether the signature was verified
    """
    pub_key = RSA.importKey(key, phrase)
    verifier = Signature_pkcs1_v1_5.new(pub_key)
    digest = SHA.new()
    digest.update(data)
    return verifier.verify(digest, base64.b64decode(signature))


def encrypt(data, key, phrase=None):
    """
    """
    pub_key = RSA.importKey(key, phrase)
    cipher = Cipher_pkcs1_v1_5.new(pub_key)
    if isinstance(data, six.text_type):
        data = data.encode('utf-8')
    cipher_text = base64.b64encode(cipher.encrypt(data))
    return cipher_text


def decrypt(data, key, phrase=None):
    """
    """
    pri_key = RSA.importKey(key, phrase)
    cipher = Cipher_pkcs1_v1_5.new(pri_key)
    return cipher.decrypt(base64.b64decode(data), pri_key)


if __name__ == '__main__':
    from Crypto import Random

    random_generator = Random.new().read
    # rsa算法生成实例

    rsa = RSA.generate(1024, random_generator)

    # master的秘钥对的生成
    private_pem = rsa.exportKey(passphrase='qdreamer')

    with open('master-private.pem', 'w') as f:
          f.write(private_pem)

    public_pem = rsa.publickey().exportKey()
    with open('master-public.pem', 'w') as f:
        f.write(public_pem)
