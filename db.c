#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
extern long long int _atoi(char *buf);
static char *meta_file_name = "/home/usaraboju/.deleted-stuff/.uma_meta_file.db";

int delete = 0;

struct globals {
    unsigned long delete;
    unsigned long restore;
    unsigned long list;
    unsigned long index;
    unsigned char *name;
};
struct record {
    unsigned char name[2*1024];
    unsigned long index;
    unsigned long pad1;
    unsigned long pad2;
    unsigned long pad3;
    unsigned long pad4;
    unsigned char pad[4096];
};

struct header {
    unsigned char magic[4];
    unsigned long how_many;
    unsigned long pad1;
    unsigned long pad2;
    unsigned long pad3;
    unsigned long pad4;
    unsigned char pad[4096];
};

struct globals global;

    int
process_args(int argc, char **argv)
{
    extern char *optarg;
    extern int optind;
    int c, err = 0;
    int rlfag=0, dflag=0, fflag=0;
    static char usage[] = "usage: %s -d filename -r index -l -h\n";

    while ((c = getopt(argc, argv, "d:r:lh")) != -1)
        switch (c) {
            case 'd':
                global.delete = 1;
                global.name = optarg;
                break;
            case 'r':
                global.restore = 1;
                global.index = _atoi(optarg);
                break;
            case 'l':
                global.list = 1;
                break;
            case 'h':
                break;
            case '?':
                err = 1;
                break;
        }
    if (1 != (global.delete + global.restore + global.list)) {
        fprintf(stderr, usage, argv[0]);
        return (1);
    } else if ((optind+1) > argc && 0) {
        return (1);
    } else if (err) {
        fprintf(stderr, usage, argv[0]);
        return (1);
    }

    if (optind < argc)	/* these are the arguments after the command-line options */
        for (; optind < argc; optind++)
            printf("argument: \"%s\"\n", argv[optind]);
    else {
        //printf("no arguments left to process\n");
    }
    return (0);
}

int open_meta_file(void)
{
    int fd = -1;

    if(access(meta_file_name, F_OK) != -1) {
        fd = open(meta_file_name, O_RDWR);
    }

    if (fd < 0)
        exit(1);

    return fd;
}

void save_offset(int fd, unsigned long long *off)
{
    *off = lseek(fd, 0, SEEK_CUR);
}

void restore_offset(int fd, unsigned long long off)
{
    lseek(fd, off, SEEK_SET);
}

void update_header(int fd)
{
    unsigned long long _off = 0;
    struct header header ;

    save_offset(fd, &_off);
    lseek(fd, 0, SEEK_SET);
    read(fd, &header, sizeof(struct header));
    header.how_many += 1;
    lseek(fd, 0, SEEK_SET);
    write(fd, &header, sizeof(struct header));
    restore_offset(fd, _off);
}
unsigned long long get_current_index(int fd)
{
    unsigned long long _off = 0;
    struct header header ;

    save_offset(fd, &_off);
    lseek(fd, 0, SEEK_SET);
    read(fd, &header, sizeof(struct header));
    restore_offset(fd, _off);
    return header.how_many;
}

int add_record(const char *file_name)
{
    int fd = open_meta_file();
    struct record record;

    lseek(fd, 0, SEEK_END);
    memset(&record, 0, sizeof(struct record));
    strncpy(record.name, file_name, strlen(file_name));
    unsigned long long cur_index = get_current_index(fd);
    record.index = cur_index;
    int out = write(fd, &record, sizeof(struct record));
    if (out != sizeof (struct record)) {
        close(fd);
        exit(1);
    }
    update_header(fd);
    close(fd);
    return 0;
}

int delete_it(const char *file_name)
{
    printf("deleting %s\n", file_name);
    add_record(file_name);
    return 0;
}

void display_record(struct record *record)
{
    printf ("index = %lu name : %s\n", record->index, record->name);
}

void print_header(int fd)
{
    unsigned long long _off = 0;
    struct header header ;

    save_offset(fd, &_off);
    lseek(fd, 0, SEEK_SET);
    read(fd, &header, sizeof(struct header));
    restore_offset(fd, _off);
    printf ("Number of files : %lu\n", header.how_many);
}

int list_them(void)
{
    int fd = open_meta_file();
    struct record record;
    int out = 0;

    print_header(fd);

    lseek(fd, sizeof(struct header), SEEK_CUR);
    while(1) {
        memset(&record, 0, sizeof(struct record));
        out = read(fd, &record, sizeof(struct record));
        if (out == 0)
            break;
        display_record(&record);
    }
    close(fd);
    return 0;
}

int validate_magic(struct header *header)
{
    if (header->magic[0] != 0xDE ||
            header->magic[1] != 0xAD ||
            header->magic[2] != 0xBE ||
            header->magic[3] != 0xEF ) {
        printf ("invalid magic value\n");
        return 1;
    }

    return 0;
}

int check_meta_file(void)
{
    int fd = -1;
    int out = -1;
    int just_created = 0;
    struct header header ;
    const size_t header_size = sizeof (struct header);

    memset(&header, 0, sizeof(struct header));
    if(access(meta_file_name, F_OK) != -1) {
        fd = open(meta_file_name, O_RDWR);
    } else {
        fd = open(meta_file_name, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
        just_created = 1;
    }

    if (fd < 0) {
        printf("opening %s failed \n", meta_file_name);
        return 1;
    }

    if (just_created) {
        header.magic[0] = 0xDE;
        header.magic[1] = 0xAD;
        header.magic[2] = 0xBE;
        header.magic[3] = 0xEF;
        out = write(fd, &header, header_size);
        if (header_size != out) {
            close (fd);
            return 1;
        }
    }else {
        if (header_size != read(fd, &header, header_size)) {
            close (fd);
            return 1;
        }
        if (validate_magic(&header)) {
            close (fd);
            return 1;
        }
    }
    close(fd);
    return 0;
}

int main(int c, char *v[])
{
    if (process_args(c, v)) {
        return 1;
    }

    if (check_meta_file()) {
        return 1;
    }

    if (global.delete) {
        delete_it(global.name);
    }else if (global.restore) {
        printf("restoring %lu\n", global.index);
    }else if (global.list) {
        list_them();
    }
    return 0;
}
