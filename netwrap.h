struct header {
	int header_size;
	int kern_size;
	int ird_size; 
	char boot_arg[200]; 
} ;

unsigned long align_pagesize(unsigned long v)
{
        return ((v + (PAGE_SIZE-1)) & ~(PAGE_SIZE-1));
}

unsigned long align_512(unsigned long v)
{
        return ((v + 511) & ~511);
}

