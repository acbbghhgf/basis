#include "qtk_mail.h"
#include <stdlib.h>

static char mail[]="python -c \"import smtplib\n\
import sys,os,socket,time\n\
from email.mime.text import MIMEText\n\
\n\
def sendMail(content):\n\
	msg=MIMEText(content)\n\
	to='li.zhang@aispeech.com'\n\
	f='li_aispeech@163.com'\n\
	msg['Subject']='http speech server'\n\
	msg['From']=f\n\
	msg['To']=to\n\
	try:\n\
		s=smtplib.SMTP()\n\
		s.connect('smtp.163.com')\n\
		s.login(f,'aispeech')\n\
		s.sendmail(f,to,msg.as_string())\n\
		s.close()\n\
		return True\n\
	except Exception,e:\n\
		import traceback\n\
		traceback.print_exc()\n\
		return False\n\
\n\
if __name__ == '__main__':\n\
	c={}\n\
	c['time']=time.asctime()\n\
	c['pwd']=os.getcwd()\n\
	c['host']=socket.gethostname()\n\
	r=['http server restart']\n\
	for k,v in c.items():\n\
		r.append('%s:\t%s' %(k,v))\n\
	for item in sys.argv:\n\
		r.append(item)\n\
	s='\\n'.join(r)\n\
	sendMail(s)\n\
\n\" &";


int qtk_send_mail()
{
	return system(mail);
}
