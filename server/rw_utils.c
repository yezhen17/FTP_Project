#include "global.h"
#include "rw_utils.h"
void get_cmd(int fd, char *message, int message_len)
{
    int p = 0;
    while (1)
    {
        int n = read(fd, message + p, message_len - p);
        if (n < 0)
        {
            printf("Error read(): %s(%d)\n", strerror(errno), errno);
            continue;
        }
        else if (n == 0)
        {
            break;
        }
        else
        {
            p += n;
            if (message[p - 1] == '\n')
            {
                break;
            }
        }
    }
    message[p - 1] = '\0';
}

// void send_response(char *response, int tmpfd)
// {
//     int len = strlen(response);
//     response[len] = '\0';
//     //printf("%d\n", len);
//     int p = 0;
//     while (p < len)
//     {
//         int n = write(tmpfd, response + p, len + 1 - p);
//         if (n < 0)
//         {
//             printf("Error write(): %s(%d)\n", strerror(errno), errno);
//             continue;
//         }
//         else
//         {
//             p += n;
//         }
//     }
// }

// int PORT_param(char *src)
// {
//     int p = 0;
//     int comma_count = 0;
//     while (src[p] != '\0')
//     {
//         if (src[p] == ',')
//         {
//             comma_count++;
//             if (comma_count == 4)
//             {
//                 break;
//             }
//             src[p] = '.';
//         }
//         p++;
//     }
//     //strncpy(dest, src, p);
//     src[p] = '\0';
//     p++;
//     int fst_hlf = 0;
//     int scd_hlf = 0;
//     while (src[p] != '\0')
//     {
//         if (src[p] == ',')
//         {
//             break;
//         }
//         else if ('0' > src[p] || '9' < src[p])
//         {
//             return -1;
//         }
//         else
//         {
//             fst_hlf *= 10;
//             fst_hlf += src[p] - '0';
//         }
//         p++;
//     }
//     p++;
//     while (src[p] != '\0')
//     {
//         if ('0' > src[p] || '9' < src[p])
//         {
//             return -1;
//         }
//         else
//         {
//             scd_hlf *= 10;
//             scd_hlf += src[p] - '0';
//         }
//         p++;
//     }
//     return (fst_hlf << 8) + scd_hlf;
// }

int strip_crlf(char *sentence, int len)
{
    int i = len - 1;
    while (sentence[i] == '\r' || sentence[i] == '\n')
    {
        sentence[i] = '\0';
        i--;
    }
    return i + 1;
}


void send_resp(int fd, int code, char *custom_resp)
{
    char resp[200];
    switch (code)
    {
    case 500:
        sprintf(resp, "%d %s\r\n", code, "Parsing error.");
        break;
    case 501:
        sprintf(resp, "%d %s\r\n", code, "Bad parameter format.");
        break;
    case 502:
        sprintf(resp, "%d %s\r\n", code, "Doesn't support this verb.");
        break;
    case 503:
        sprintf(resp, "%d %s\r\n", code, "Previous command is not USER.");
        break;
    case 504:
        sprintf(resp, "%d %s\r\n", code, "Doesn't support this parameter.");
        break;
    case 530:
        sprintf(resp, "%d %s\r\n", code, "Permission denied.");
        break;
    case 550:
        sprintf(resp, "%d %s\r\n", code, "No such file or directory.");
        break;
    case 200:
        sprintf(resp, "%d %s\r\n", code, custom_resp);
        break;
    case 215:
        sprintf(resp, "%d %s\r\n", code, "UNIX Type: L8");
        break;
    case 220:
        sprintf(resp, "%d %s\r\n", code, "DIY FTP server ready.");
        break;
    case 221:
        sprintf(resp, "%d %s\r\n", code, "Goodbye.");
        break;
    case 226:
        sprintf(resp, "%d %s\r\n", code, "Transfer complete.");
        break;
    case 230:
        sprintf(resp, "%d %s\r\n", code, "Permission granted.");
        break;
    case 250:
        sprintf(resp, "%d %s\r\n", code, "Okay.");
        break;
    case 331:
        sprintf(resp, "%d %s\r\n", code, "Guest login ok, send your complete e-mail address as password.");
        break;
    case 350:
        sprintf(resp, "%d %s\r\n", code, "File exists.");
        break;
    case 425:
        sprintf(resp, "%d %s\r\n", code, "Please establish a connection first.");
        break;
    case 426:
        sprintf(resp, "%d %s\r\n", code, "network failure, try again?");
        break;
    case 451:
        sprintf(resp, "%d %s\r\n", code, "Failed to read file.");
        break;
    case 150:
        sprintf(resp, "%d %s\r\n", code, "Opening connection.");
        break;
    default:
        sprintf(resp, "%d %s\r\n", code, custom_resp);
        break;
    }
    int len = strlen(resp);
    send(fd, resp, len, MSG_WAITALL);
    return;
}

int safe_send(int fd, char *buf, int len)
{
    int p = 0;
    int n;
    while (p < len)
    {
        n = write(fd, buf + p, len - p);
        if (n < 0)
        {
            printf("Error write(): %s(%d)\n", strerror(errno), errno);
            return 0;
        }
        else if (n == 0)
        {
            break;
        }
        else
        {
            p += n;
        }
    }
    return 1;
}

int safe_recv(int fd, char *buf, int len)
{
    int p = 0;
    int n;
    while (1)
    {
        n = read(fd, buf + p, len - p);
        if (n < 0)
        {
            printf("Error read(): %s(%d)\n", strerror(errno), errno); //read����֤һ�ζ��꣬������;�˳�
            return 0;
        }
        else if (n == 0)
        {
            break;
        }
        else
        {
            p += n;
        }
    }
    return p;
}