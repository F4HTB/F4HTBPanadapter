#include <dirent.h>

int indexlistofcolorfile = 0,indexlistofcolorfilemax=0;
char listofcolorfile[20][20];

short int colormap_rainbow[256][3] = {
  { 0, 0, 0 },
  { 45, 0, 36 },
  { 56, 0, 46 },
  { 60, 0, 49 },
  { 67, 0, 54 },
  { 70, 0, 59 },
  { 71, 0, 61 },
  { 75, 0, 68 },
  { 74, 0, 73 },
  { 74, 0, 77 },
  { 73, 0, 81 },
  { 71, 0, 87 },
  { 69, 1, 90 },
  { 68, 2, 94 },
  { 66, 3, 97 },
  { 63, 6, 102 },
  { 61, 7, 106 },
  { 58, 10, 109 },
  { 56, 12, 113 },
  { 53, 15, 116 },
  { 48, 18, 119 },
  { 47, 20, 121 },
  { 44, 23, 124 },
  { 41, 27, 128 },
  { 40, 28, 129 },
  { 37, 32, 132 },
  { 34, 36, 134 },
  { 29, 43, 137 },
  { 25, 52, 138 },
  { 24, 57, 139 },
  { 24, 62, 141 },
  { 24, 64, 142 },
  { 23, 65, 142 },
  { 23, 69, 143 },
  { 23, 71, 142 },
  { 23, 71, 142 },
  { 23, 73, 142 },
  { 23, 75, 142 },
  { 23, 75, 142 },
  { 23, 78, 142 },
  { 23, 80, 142 },
  { 23, 80, 142 },
  { 23, 82, 141 },
  { 23, 85, 141 },
  { 23, 85, 141 },
  { 23, 87, 140 },
  { 23, 87, 140 },
  { 24, 90, 140 },
  { 24, 90, 140 },
  { 24, 93, 139 },
  { 24, 93, 139 },
  { 24, 93, 139 },
  { 24, 93, 139 },
  { 24, 97, 139 },
  { 24, 97, 139 },
  { 25, 101, 138 },
  { 25, 101, 138 },
  { 25, 104, 137 },
  { 25, 104, 137 },
  { 25, 104, 137 },
  { 26, 108, 137 },
  { 26, 108, 137 },
  { 27, 111, 136 },
  { 27, 111, 136 },
  { 27, 111, 136 },
  { 27, 115, 135 },
  { 27, 115, 135 },
  { 28, 118, 134 },
  { 28, 118, 134 },
  { 29, 122, 133 },
  { 29, 122, 133 },
  { 29, 122, 133 },
  { 29, 122, 133 },
  { 29, 125, 132 },
  { 29, 125, 132 },
  { 30, 128, 131 },
  { 30, 128, 131 },
  { 31, 131, 130 },
  { 31, 131, 130 },
  { 31, 131, 130 },
  { 32, 134, 128 },
  { 32, 134, 128 },
  { 33, 137, 127 },
  { 33, 137, 127 },
  { 33, 137, 127 },
  { 34, 140, 125 },
  { 34, 140, 125 },
  { 35, 142, 123 },
  { 35, 142, 123 },
  { 36, 145, 121 },
  { 36, 145, 121 },
  { 36, 145, 121 },
  { 37, 147, 118 },
  { 37, 147, 118 },
  { 38, 150, 116 },
  { 38, 150, 116 },
  { 40, 152, 113 },
  { 40, 152, 113 },
  { 41, 154, 111 },
  { 41, 154, 111 },
  { 42, 156, 108 },
  { 42, 156, 108 },
  { 43, 158, 106 },
  { 43, 158, 106 },
  { 43, 158, 106 },
  { 45, 160, 104 },
  { 45, 160, 104 },
  { 46, 162, 101 },
  { 46, 162, 101 },
  { 48, 164, 99 },
  { 48, 164, 99 },
  { 50, 166, 97 },
  { 50, 166, 97 },
  { 51, 168, 95 },
  { 53, 170, 93 },
  { 53, 170, 93 },
  { 53, 170, 93 },
  { 55, 172, 91 },
  { 55, 172, 91 },
  { 57, 174, 88 },
  { 57, 174, 88 },
  { 59, 175, 86 },
  { 62, 177, 84 },
  { 64, 178, 82 },
  { 64, 178, 82 },
  { 67, 180, 80 },
  { 67, 180, 80 },
  { 69, 181, 79 },
  { 72, 183, 77 },
  { 72, 183, 77 },
  { 72, 183, 77 },
  { 75, 184, 76 },
  { 77, 186, 74 },
  { 80, 187, 73 },
  { 83, 189, 72 },
  { 87, 190, 72 },
  { 91, 191, 71 },
  { 95, 192, 70 },
  { 99, 193, 70 },
  { 103, 194, 70 },
  { 107, 195, 70 },
  { 111, 196, 70 },
  { 111, 196, 70 },
  { 115, 196, 70 },
  { 119, 197, 70 },
  { 123, 197, 70 },
  { 130, 198, 71 },
  { 133, 199, 71 },
  { 137, 199, 72 },
  { 140, 199, 72 },
  { 143, 199, 73 },
  { 143, 199, 73 },
  { 147, 199, 73 },
  { 150, 199, 74 },
  { 153, 199, 74 },
  { 156, 199, 75 },
  { 160, 200, 76 },
  { 167, 200, 78 },
  { 170, 200, 79 },
  { 173, 200, 79 },
  { 173, 200, 79 },
  { 177, 200, 80 },
  { 180, 200, 81 },
  { 183, 199, 82 },
  { 186, 199, 82 },
  { 190, 199, 83 },
  { 196, 199, 85 },
  { 199, 198, 85 },
  { 199, 198, 85 },
  { 203, 198, 86 },
  { 206, 197, 87 },
  { 212, 197, 89 },
  { 215, 196, 90 },
  { 218, 195, 91 },
  { 224, 194, 94 },
  { 224, 194, 94 },
  { 230, 193, 96 },
  { 233, 192, 98 },
  { 236, 190, 100 },
  { 238, 189, 104 },
  { 240, 188, 106 },
  { 240, 188, 106 },
  { 242, 187, 110 },
  { 244, 185, 114 },
  { 245, 184, 116 },
  { 247, 183, 120 },
  { 248, 182, 123 },
  { 248, 182, 123 },
  { 250, 181, 125 },
  { 251, 180, 128 },
  { 252, 180, 130 },
  { 253, 180, 133 },
  { 253, 180, 133 },
  { 254, 180, 134 },
  { 254, 179, 138 },
  { 255, 179, 142 },
  { 255, 179, 145 },
  { 255, 179, 145 },
  { 255, 179, 152 },
  { 255, 180, 161 },
  { 255, 180, 164 },
  { 255, 180, 167 },
  { 255, 180, 167 },
  { 255, 181, 169 },
  { 255, 181, 170 },
  { 255, 182, 173 },
  { 255, 183, 176 },
  { 255, 183, 176 },
  { 255, 184, 179 },
  { 255, 185, 179 },
  { 255, 185, 182 },
  { 255, 186, 182 },
  { 255, 186, 182 },
  { 255, 187, 185 },
  { 255, 188, 185 },
  { 255, 189, 188 },
  { 255, 189, 188 },
  { 255, 190, 188 },
  { 255, 191, 191 },
  { 255, 192, 191 },
  { 255, 194, 194 },
  { 255, 194, 194 },
  { 255, 197, 197 },
  { 255, 198, 198 },
  { 255, 200, 200 },
  { 255, 201, 201 },
  { 255, 201, 201 },
  { 255, 202, 202 },
  { 255, 203, 203 },
  { 255, 205, 205 },
  { 255, 206, 206 },
  { 255, 206, 206 },
  { 255, 208, 208 },
  { 255, 209, 209 },
  { 255, 211, 211 },
  { 255, 215, 215 },
  { 255, 216, 216 },
  { 255, 216, 216 },
  { 255, 218, 218 },
  { 255, 219, 219 },
  { 255, 221, 221 },
  { 255, 223, 223 },
  { 255, 226, 226 },
  { 255, 228, 228 },
  { 255, 230, 230 },
  { 255, 230, 230 },
  { 255, 232, 232 },
  { 255, 235, 235 },
  { 255, 237, 237 },
  { 255, 240, 240 },
  { 255, 243, 243 },
  { 255, 246, 246 },
  { 255, 249, 249 },
  { 255, 251, 251 },
  { 255, 253, 253 },
  { 255, 255, 255 },
};



void scandirfilecolor()
/* Scans a directory and retrieves all files of given extension */
{
    DIR *d = NULL;
    struct dirent *dir = NULL;
	int ret,i=0;
	char *p;
    d = opendir(".");
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
			
            p=strtok(dir->d_name,".");
            p=strtok(NULL,".");
            if(p!=NULL)
            {
                ret=strcmp(p,"256");
                if(ret==0)
                {
            
				printf("%s\n",dir->d_name);
				strcpy( listofcolorfile[i], dir->d_name);
				indexlistofcolorfilemax=i;
				i++;
			 }
        }
		}
        closedir(d);
   } 
}



void read_csv(char *filename){
	FILE *file;
	file = fopen(filename, "r");
	int i = 0;
    char line[15];
	while (fgets(line, 15, file) && (i < 256))
    {
        char* tmp = strdup(line);

		const char* tok;
		tok = strtok(tmp, ";");
		colormap_rainbow[i][0]=(short int)atof(tok);
		tok = strtok(NULL, ";");
		colormap_rainbow[i][1]=(short int)atof(tok);
		tok = strtok(NULL, ";");
		colormap_rainbow[i][2]=(short int)atof(tok);
   
		//printf("%d %d %d\n", colormap_rainbow[i][0],colormap_rainbow[i][1],colormap_rainbow[i][2]);

        free(tmp);
        i++;
    }
}