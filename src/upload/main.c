
extern void UploadToServer(const char*type,const char *value);

int main(int argc,char **argv)
{
	if(argc!=3)
		return -1;
	UploadToServer(argv[1],argv[2]);
	return 0;
}



