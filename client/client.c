#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>
#include <errno.h>

#include <fcntl.h>
#include <string.h>
#include <memory.h>
#include <stdio.h>

int main(int argc, char **argv) {
	int sockfd;
	struct sockaddr_in addr;
	char sentence[8192];
	int len;
	int p;

	//����socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	//����Ŀ��������ip��port
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(6789);
	if (inet_pton(AF_INET, "192.168.11.131", &addr.sin_addr) <= 0) {			//ת��ip��ַ:���ʮ����-->������
		printf("Error inet_pton(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	//������Ŀ����������socket��Ŀ���������ӣ�-- ��������
	if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		printf("Error connect(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}
    while(1) {
        //��ȡ��������
		fgets(sentence, 4096, stdin);

		len = strlen(sentence);
		// sentence[len] = '\n';
		// sentence[len + 1] = '\0';
		int test = 0;
	    if(strncmp(sentence, "RETR", 4) == 0) {
			test = 1;
		}
		printf("%d", test);
		//�Ѽ�������д��socket
		p = 0;
		while (p < len) {
			int n = write(sockfd, sentence + p, len - p);		//write��������֤���е�����д�꣬������;�˳�
			if (n < 0) {
				printf("Error write(): %s(%d)\n", strerror(errno), errno);
				return 1;
 			} else {
				p += n;
			}			
		}

		//ե��socket���յ�������
		p = 0;
		while (1) {
			int n = read(sockfd, sentence + p, 8191 - p);
			if (n < 0) {
				printf("Error read(): %s(%d)\n", strerror(errno), errno);	//read����֤һ�ζ��꣬������;�˳�
				return 1;
			} else if (n == 0) {
				break;
			} else {
				p += n;
				if (sentence[p - 1] == '\0') {
					break;
				}
			}
		}
        //ע�⣺read�����Ὣ�ַ�������'\0'����Ҫ�ֶ�����
		sentence[p - 1] = '\0';

		printf("FROM SERVER: %s", sentence);

		if (test == 1) {
			fflush(stdout);
			printf("start listening at 2560\n");
			int listenfd;
			struct sockaddr_in addr;
    		//����socket
    		if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    		{
        		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
        		return -1;
    		}

    		//���ñ�����ip��port
    		memset(&addr, 0, sizeof(addr));
    		addr.sin_family = AF_INET;
    		addr.sin_port = htons(2560);
    		addr.sin_addr.s_addr = htonl(INADDR_ANY); //����"0.0.0.0"

    		//��������ip��port��socket��
    		if (bind(listenfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    		{
    		    printf("Error bind(): %s(%d)\n", strerror(errno), errno);
    		    return -1;
    		}

    		//��ʼ����socket
    		if (listen(listenfd, 10) == -1)
    		{
    		    printf("Error listen(): %s(%d)\n", strerror(errno), errno);
    		    return -1;
    		}
			printf("listening at 2560, %d\n", listenfd);
            int transfd;
			if ((transfd = accept(listenfd, NULL, NULL)) == -1) {
                printf("Error accept(): %s(%d)\n", strerror(errno), errno);
            }
			printf("accepted, %d\n", transfd);
			int f;
			if ((f = open("res.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0)
			{
				printf("Open Error!\n");
				exit(1);
			}
			int nbytes;
			char buffer[8192];
			//read�������׽����л�ȡN�ֽ����ݷ���buffer�У�����ֵΪ��ȡ���ֽ���
			while ((nbytes = read(transfd, buffer, 8192)) > 0)
			{
				printf("%d", nbytes);
				//write������buffe0r�е����ݶ�ȡ����д��fd��ָ����ļ�������ֵΪʵ��д����ֽ���
				if (write(f, buffer, nbytes) < 0)
				{
					printf("Write Error!At commd_get 2");
				}
			}

			close(f);
			close(transfd);

			//printf("FROM SERVER: %s", sentence);
		}

		
	}
	

	close(sockfd);

	return 0;
}
