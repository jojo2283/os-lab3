#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define DEVICE_FILE "/dev/vmstat_ioctl"
#define IOCTL_GET_VMSTAT _IOR('v', 1, char *)

int main() {
    int fd;
    char buffer[256];

    fd = open(DEVICE_FILE, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open device");
        return 1;
    }

    if (ioctl(fd, IOCTL_GET_VMSTAT, buffer) < 0) {
        perror("Failed to get vmstat");
        close(fd);
        return 1;
    }

    printf("VMStat Information:\n%s", buffer);

    close(fd);
    return 0;
}
