#ifndef _LIBRARY_H
#define _LIBRARY_H

int read_disklabel(int fd,struct disklabel *d);
int dosumlabel(int fd,struct disklabel *d);
int overlaplabel(struct disklabel *d,int offset,int endplace, unsigned force);

#endif
