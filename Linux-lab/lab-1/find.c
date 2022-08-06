#include <unistd.h>
#include <stdio.h>
#include<stdlib.h>
#include<string.h>
/*
	将路径格式化为文件名
*/
char *fmtname(char *path)
{
    char *p;
    for (p = path + strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;
    return p;
}
void equal_print(char *path, char *findname)
{
    if (strcmp(fmtname(path), findname) == 0)
        printf("%s\n", path);
}

void find(char *dir_name, char *file_name)
{
    int fd;

    if ((fd = open(dir_name, 0)) < 0)
    {
        fprintf(2, "ls: cannot open %s\n", dir_name);
        return;
    }

    struct stat st;
    if (fstat(fd, &st) < 0)
    {
        fprintf(2, "ls: cannot stat %s\n", dir_name);
        close(fd);
        return;
    }

    //循环判断
    struct dirent de;
    //buf是用来记录文件前缀的，这样才会打印出之前的目录
    char buf[512], *p;
    switch (st.type)
    {
    case T_FILE:
        equal_print(dir_name, file_name);
        break;
    case T_DIR:
        if (strlen(dir_name) + 1 + DIRSIZ + 1 > sizeof buf)
        {
            printf("find: path too long\n");
            break;
        }
        //将path复制到buf里
        strcpy(buf, dir_name);
        //p为一个指针，指向buf(path)的末尾
        p = buf + strlen(buf);
        //在末尾添加/ 比如 path为 a/b/c 经过这步后变为 a/b/c/<-p
        *p++ = '/';
        // 如果是文件夹，则循环读这个文件夹里面的文件
        while (read(fd, &de, sizeof(de)) == sizeof(de))
        {
            if (de.inum == 0 || (strcmp(de.name, ".") == 0) || (strcmp(de.name, "..") == 0))
                continue;
            //拼接出形如 a/b/c/de.name 的新路径(buf)
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;
            //递归查找
            find(buf, file_name);
        }
        break;
    }
    close(fd);
}

int main(int argc, char *argv[])
{

    if (argc != 3)
    {
        printf("Usage: find <dirName> <fileName>\n");
        exit(-1);
    }

    find(argv[1], argv[2]);
    exit(0);
}