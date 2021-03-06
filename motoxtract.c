/*
 * motoxtract.c
 *
 * Copyright (c) Ramsudharsan <ramsudharsanm@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

int usage(char *name) {
    printf("usage:\t%s <path_to_motoboot.img_file>\n", name);
    return 0;
}

struct entry {
    char partitionName[24];
    uint32_t firstSector;
    uint32_t lastSector;
};

struct moto_header {
    uint32_t partitionNumber;
    struct entry partitions[30];
    char padding[28];
}header;

int main (int argc, char **argv) {
    FILE *motoboot, *partitionDump;
    unsigned char buff[1024];
    int partitionCount, i, j;
    char partitionName[32];

    if (argc != 2)
        return usage(argv[0]);

    motoboot = fopen(argv[1], "r");

    if (motoboot == NULL) {
        printf("unable to open %s file, abort\n", argv[1]);
        return -1;
    }
    printf("reading file %s\n", argv[1]);

    fread((void*)buff, 1024, 1, motoboot);

    memcpy(&header, buff, sizeof(header));
    partitionCount = header.partitionNumber;

    if (partitionCount > 30) {
        printf("error reading the file, too many partitions\n");
        return -1;
    }
    printf("found %d partititions\n", partitionCount);

    for (i = 0; i < partitionCount; i++) {
        sprintf(partitionName, "%s.mbn", header.partitions[i].partitionName);
        partitionDump = fopen(partitionName, "w+");
        if (partitionDump == NULL) {
            printf("unable to write to file %s", partitionName);
            return -1;
        }
        uint32_t sectors = header.partitions[i].lastSector - header.partitions[i].firstSector + 1;
        printf("dumping partition %d (%10s), starting offset in file 0x%08X\n", i, header.partitions[i].partitionName, 1024 + header.partitions[i].firstSector * 512);
        for (j = 0; j < sectors; j++) {
            fread((void*)buff, 512, 1, motoboot);
            fwrite((void*)buff, 512, 1, partitionDump);
        }
        fclose(partitionDump);
    }

    fclose(motoboot);
    return 0;
}
