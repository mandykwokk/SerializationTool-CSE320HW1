
#include "const.h"
#include "helper.h"
#include "transplant.h"
#include "debug.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

/*
 * You may modify this file and/or move the functions contained here
 * to other source serialize_filees (except for main.c) as you wish.
 *
 * IMPORTANT: You MAY NOT use any array brackets (i.e. [ and ]) and
 * you MAY NOT declare any arrays or allocate any storage with malloc().
 * The purpose of this restriction is to force you to use pointers.
 * Variables to hold the pathname of the current file or directory
 * as well as other data have been pre-declared for you in const.h.
 * You must use those variables, rather than declaring your own.
 * IF YOU VIOLATE THIS RESTRICTION, YOU WILL GET A ZERO!
 *
 * IMPORTANT: You MAY NOT use floating point arithmetic or declare
 * any "float" or "double" variables.  IF YOU VIOLATE THIS RESTRICTION,
 * YOU WILL GET A ZERO!
 */

/*
 * A function that returns printable names for the record types, for use in
 * generating debugging printout.

 */

static char *record_type_name(int i) {
    switch(i) {
    case START_OF_TRANSMISSION:
	return "START_OF_TRANSMISSION";
    case END_OF_TRANSMISSION:
	return "END_OF_TRANSMISSION";
    case START_OF_DIRECTORY:
	return "START_OF_DIRECTORY";
    case END_OF_DIRECTORY:
	return "END_OF_DIRECTORY";
    case DIRECTORY_ENTRY:
	return "DIRECTORY_ENTRY";
    case FILE_DATA:
	return "FILE_DATA";
    default:
	return "UNKNOWN";
    }
}

/*
 * @brief  Initialize path_buf to a specified base path.
 * @details  This function copies its null-terminated argument string into
 * path_buf, including its terminating null byte.
 * The function fails if the argument string, including the terminating
 * null byte, is longer than the size of path_buf.  The path_length variable
 * is set to the length of the string in path_buf, not including the terminating
 * null byte.
 *
 * @param  Pathname to be copied into path_buf.
 * @return 0 on success, -1 in case of error
 */
int path_init(char *name) {
    char *ptr = name;
    char *bufptr = path_buf;//path_length not include null termin
    int count = 0;
    while(*ptr!='\0' && count<PATH_MAX-1){
        *bufptr = *ptr;
        bufptr++;
        ptr++;
        count++;
    }
    if(*ptr!='\0')
        return -1;
    *bufptr = '\0';
    path_length = count;
    return 0;
}

/*
 * @brief  Append an additional component to the end of the pathname in path_buf.
 * @details  This function assumes that path_buf has been initialized to a valid
 * string.  It appends to the existing string the path separator character '/',
 * followed by the string given as argument, including its terminating null byte.
 * The length of the new string, including the terminating null byte, must be
 * no more than the size of path_buf.  The variable path_length is updated to
 * remain consistent with the length of the string in path_buf.
 *
 * @param  The string to be appended to the path in path_buf.  The string must
 * not contain any occurrences of the path separator character '/'.
 * @return 0 in case of success, -1 otherwise.
 */
int path_push(char *name) {
    int spaces = PATH_MAX - path_length - 1;//null and /
    int valid = checkName(name);
    if(valid==-1||valid>spaces)
        return -1;
    char* bufptr = path_buf;
    while(*bufptr!='\0'){
        bufptr++;
    }
    char* ptr = name;
    if(*path_buf!='\0'){
        *bufptr = '/';
        bufptr++;
    }
    while(*ptr!='\0'){
        *bufptr = *ptr;
        ptr++;
        bufptr++;
    }
    return 0;
}

int checkName(char *name){
    int length = 0;
    char* ptr = name;
    while(*ptr!='\0'){
        if (*ptr=='/')
            return -1;
        ptr++;
        length++;
    }
    return length+1;//include null byte
}

/*
 * @brief  Remove the last component from the end of the pathname.
 * @details  This function assumes that path_buf contains a non-empty string.
 * It removes the suffix of this string that starts at the last occurrence
 * of the path separator character '/'.  If there is no such occurrence,
 * then the entire string is removed, leaving an empty string in path_buf.
 * The variable path_length is updated to remain consistent with the length
 * of the string in path_buf.  The function fails if path_buf is originally
 * empty, so that there is no path component to be removed.
 *
 * @return 0 in case of success, -1 otherwise.
 */
int path_pop() {
    if (*path_buf=='\0')
        return -1;
    char* bufptr = path_buf;
    while(*bufptr!='\0'){
        bufptr++;
    } //last position
    while(bufptr!=path_buf){
        if(*bufptr=='/')
            break;
        *bufptr = '\0';
        bufptr--;
        path_length--;
    }
    *bufptr = '\0';
    return 0;
}

/*
 * @brief Deserialize directory contents into an existing directory.
 * @details  This function assumes that path_buf contains the name of an existing
 * directory.  It reads (from the standard input) a sequence of DIRECTORY_ENTRY
 * records bracketed by a START_OF_DIRECTORY and END_OF_DIRECTORY record at the
 * same depth and it recreates the entries, leaving the deserialized files and
 * directories within the directory named by path_buf.
 *
 * @param depth  The value of the depth field that is expected to be found in
 * each of the records processed.
 * @return 0 in case of success, -1 in case of an error.  A variety of errors
 * can occur, including depth fields in the records read that do not match the
 * expected value, the records to be processed to not being with START_OF_DIRECTORY
 * or end with END_OF_DIRECTORY, or an I/O error occurs either while reading
 * the records from the standard input or in creating deserialized files and
 * directories.
 */
int deserialize_directory(int depth) {
    if(depth<previousDepth||currentSize!=16)
        return -1;
    if(previousType==START_OF_TRANSMISSION){ //0
        DIR* dir = opendir(path_buf);
        if(dir){//exists
            if(global_options<=8)//no clobber
                return -1;
        }
        else{//not exist
            if(*path_buf == '\0'){
                closedir(dir);
                count2++;
                return 0;
            }
            if(mkdir(path_buf,0700)==-1)//0700? currentSTmode?
                return -1;
        }
        closedir(dir);
    }
    else if(previousType==DIRECTORY_ENTRY){ //4
        if(depth-previousDepth!=1)
            return -1;
        if(path_push(name_buf)==-1){//path buf updated, path length
            return -1;
        }
        DIR* dir = opendir(path_buf);
        if(dir){//exists
            if(global_options<=8)//no clobber
                return -1;
        }
        else{//not exist
            if(mkdir(path_buf,0700)==-1)//0700? currentSTmode?
                return -1;
        }
        closedir(dir);
        if(stat(path_buf, &stat_buf)!=-1){
            chmod(path_buf, currentSTmode & 0777);
            stat_buf.st_mode = currentSTmode;
            stat_buf.st_size = currentSTsize;
        }
        else
            return -1;
    }
    else
        return -1;
    count2++;
    return 0;
}

/*
 * @brief Deserialize the contents of a single file.
 * @details  This function assumes that path_buf contains the name of a file
 * to be deserialized.  The file must not already exist, unless the ``clobber''
 * bit is set in the global_options variable.  It reads (from the standard input)
 * a single FILE_DATA record containing the file content and it recreates the file
 * from the content.
 *
 * @param depth  The value of the depth field that is expected to be found in
 * the FILE_DATA record.
 * @return 0 in case of success, -1 in case of an error.  A variety of errors
 * can occur, including a depth field in the FILE_DATA record that does not match
 * the expected value, the record read is not a FILE_DATA record, the file to
 * be created already exists, or an I/O error occurs either while reading
 * the FILE_DATA record from the standard input or while re-creating the
 * deserialized file.
 */
int deserialize_file(int depth){
    int doc_size = currentSize-16;
    if(doc_size<0)
        return -1;
    if(previousType!=DIRECTORY_ENTRY||previousDepth!=depth)
        return -1;
    if(path_push(name_buf)==-1)//path buf updated, path length
        return -1;
    FILE *f = fopen(path_buf,"r");
    if(f){//exists
        if(global_options>8)//clobber
            f = fopen(path_buf,"w+");
        else
            return -1;//no -c
    }
    else
        f = fopen(path_buf,"w");
    if(f==NULL)
        return -1;
    for(int i=0;i<doc_size;i++){
        int c = getchar();
        if(c==EOF||c==-1)
            return -1;
        if(fputc(c,f)==EOF)
            return -1;
    }
   // chmod(path_buf, currentSTmode & 0777);
    if(fclose(f)==-1)
        return -1;
    if(stat(path_buf, &stat_buf)!=-1){
        chmod(path_buf, currentSTmode & 0777);
        stat_buf.st_mode = currentSTmode;
        stat_buf.st_size = currentSTsize;
    }
    else
        return -1;
    if(path_pop()==-1)
        return -1;
    count5++;
    return 0;
}

/*
 * @brief  Serialize the contents of a directory as a sequence of records written
 * to the standard output.
 * @details  This function assumes that path_buf contains the name of an existing
 * directory to be serialized.  It serializes the contents of that directory as a
 * sequence of records that begins with a START_OF_DIRECTORY record, ends with an
 * END_OF_DIRECTORY record, and with the intervening records all of type DIRECTORY_ENTRY.
 *
 * @param depth  The value of the depth field that is expected to occur in the
 * START_OF_DIRECTORY, DIRECTORY_ENTRY, and END_OF_DIRECTORY records processed.
 * Note that this depth pertains only to the "top-level" records in the sequence:
 * DIRECTORY_ENTRY records may be recursively followed by similar sequence of
 * records describing sub-directories at a greater depth.
 * @return 0 in case of success, -1 otherwise.  A variety of errors can occur,
 * including failure to open files, failure to traverse directories, and I/O errors
 * that occur while reading file content and writing to standard output.
 */

int serialize_directory(int depth) {
    DIR *dir = opendir(path_buf);
    struct dirent *de;
   // printf("path_buf %s\n", path_buf);
    putHeader(2,depth,16);
    while((de = readdir(dir)) !=NULL){
        //printf("path_buf %s\n", path_buf);
        char* name = de->d_name;
        if(*name=='.'||(*name=='.'&&*(name+1)=='.'))
            continue;
        if(path_push(name)==-1)
            return -1;
        //printf("path_buf %s\n", path_buf);
        stat(path_buf,&stat_buf);
        if(S_ISREG(stat_buf.st_mode)){ //file
            putHeader(4,depth,28+length(name));//12+length
            //metadata
            putFour(stat_buf.st_mode);
            putEight(stat_buf.st_size);
            char* ptr = name;
            while(*ptr!='\0'){
                putchar(*ptr);
                ptr++;
            }
            if(serialize_file(depth,stat_buf.st_size))
                return -1;
            if(path_pop()==-1)
                return -1;
        }
        else if(S_ISDIR(stat_buf.st_mode)){ //common case
          //  printf("path_buf: %s\n", path_buf);
            putHeader(4,depth,28+length(name));
            //metadata
            putFour(stat_buf.st_mode);
            putEight(stat_buf.st_size);
            char* ptr = name;
            while(*ptr!='\0'){
                putchar(*ptr);
                ptr++;
            }
            if(serialize_directory(depth+1)==-1)
                return -1;
            if(path_pop()==-1)
                return -1;
        }
        else{//not exist or no path
            return -1;
        }
    }
    closedir(dir);
    putHeader(3,depth,16);
    return 0;
}

int length(char* string){
    int count = 0;
    char* ptr = string;
    while(*ptr!='\0'){
        count++;
        ptr++;
    }
    return count;
}

/*
 * @brief  Serialize the contents of a file as a single record written to the
 * standard output.
 * @details  This function assumes that path_buf contains the name of an existing
 * file to be serialized.  It serializes the contents of that file as a single
 * FILE_DATA record emitted to the standard output.
 *
 * @param depth  The value to be used in the depth field of the FILE_DATA record.
 * @param size  The number of bytes of data in the file to be serialized.
 * @return 0 in case of success, -1 otherwise.  A variety of errors can occur,
 * including failure to open the file, too many or not enough data bytes read
 * from the file, and I/O errors reading the file data or writing to standard output.
 */
int serialize_file(int depth, off_t size) {
    putHeader(5,depth,size+16);
    FILE *f = fopen(path_buf,"r");
    if(f){ //exists
        char c;
        while((c=fgetc(f))!=EOF){
            putchar(c);
        }
    }
    else//not exist
        return -1;
    return 0;
}

/**
 * @brief Serializes a tree of files and directories, writes
 * serialized data to standard output.
 * @details This function assumes path_buf has been initialized with the pathname
 * of a directory whose contents are to be serialized.  It traverses the tree of
 * files and directories contained in this directory (not including the directory
 * itself) and it emits on the standard output a sequence of bytes from which the
 * tree can be reconstructed.  Options that modify the behavior are obtained from
 * the global_options variable.
 *
 * @return 0 if serialization completes without error, -1 if an error occurs.
 */
int serialize() {
    putHeader(0,0,16);
    stat(path_buf,&stat_buf);
    if(S_ISREG(stat_buf.st_mode)){ //file
        if(serialize_file(1,stat_buf.st_size)==-1)
            return -1;
    }
    else if(S_ISDIR(stat_buf.st_mode)){ //common case
        if(serialize_directory(1)==-1)
            return -1;
    }
    else{//not exist or -s
        if(*path_buf=='\0'){
            path_init("./");
            if(serialize_directory(1)==-1)
                return -1;
        }
        else
            return -1;
    }
    putHeader(1,0,16);
    fflush(stdout);
    return 0;
}

void putMagic(){
    putchar(0x0c);
    putchar(0x0d);
    putchar(0xed);
}

void putFour(int depth){
    putchar((depth & 0xff000000)>>24);
    putchar((depth & 0xff0000)>>16);
    putchar((depth & 0xff00)>>8);
    putchar((depth & 0xff));
}

void putEight(int size){
    putchar((size & 0xff00000000000000)>>56);
    putchar((size & 0xff000000000000)>>48);
    putchar((size & 0xff0000000000)>>40);
    putchar((size & 0xff00000000)>>32);
    putchar((size & 0xff000000)>>24);
    putchar((size & 0xff0000)>>16);
    putchar((size & 0xff00)>>8);
    putchar((size & 0xff));
}

void putHeader(int type,int depth, int size){
    putMagic();
    putchar(type);
    putFour(depth);
    putEight(size);
}



/**
 * @brief Reads serialized data from the standard input and reconstructs from it
 * a tree of files and directories.
 * @details  This function assumes path_buf has been initialized with the pathname
 * of a directory into which a tree of files and directories is to be placed.
 * If the directory does not already exist, it is created.  The function then reads
 * from from the standard input a sequence of bytes that represent a serialized tree
 * of files and directories in the format written by serialize() and it reconstructs
 * the tree within the specified directory.  Options that modify the behavior are
 * obtained from the global_options variable.
 *
 * @return 0 if deserialization completes without error, -1 if an error occurs.
 */
int deserialize() {
    while(processHeader()!=-1){
        if(currentType==START_OF_TRANSMISSION){ //0
            //printf("entered0\n");
            if(currentDepth!=0||currentSize!=16)
                return -1;
            count0++;
            if(count0!=1)
                return -1;
        }
        else if(currentType==END_OF_TRANSMISSION){ //1
           // printf("entered1\n");
            if(currentDepth!=0||currentSize!=16)
                return -1;
            count1++;
            if(count1!=1)
                return -1;
        }
        else if(currentType==START_OF_DIRECTORY){ //2
            //printf("ST mode %d\n", currentSTmode);
            //printf("ST size %d\n", currentSTsize);
            //printf("name_buf %s\n", name_buf);
            //printf("entered2\n");
            if(deserialize_directory(currentDepth)==-1)
                return -1;
        }
        else if(currentType==END_OF_DIRECTORY){ //3
            //printf("entered3\n");
            if(currentSize!=16)
                return -1;
            path_pop();
               // return -1;
            count3++;
        }
        else if(currentType==DIRECTORY_ENTRY){ //4
            //printf("entered4\n");
            int datalength = currentSize-16; //31-15
            int nameLength = datalength-12; //3 for dir
            if(nameLength<=0||datalength<=12)
                return -1;//start metadata first 4 bytes st_mode
            currentSTmode = 0;
            for(int i=6;i>=0;i-=2){//stat("hi.txt",&stat_buf);
                int x = getchar();
                if(x==EOF)
                    return -1;
                currentSTmode+=(x*power(16,i));
            }
            currentSTsize = 0;
            for(int i=14;i>=0;i-=2){
                int x = getchar();
                if(x==EOF)
                    return -1;
                currentSTsize+=(x*power(16,i));
            }//initialize namebuf
            clearNameBuf();
            char* name_ptr = name_buf;
            for(int i=0;i<nameLength;i++){
                int x = getchar();
                if(x==EOF)
                    return -1;
                *name_ptr = x;
                name_ptr++;
            }
          //  printf("name %s\n",name_buf );
            count4++;
        }
        else if(currentType==FILE_DATA){ //5
           // printf("entered5\n");
            if(deserialize_file(currentDepth)==-1)
                return -1;
         //   printf("return 0\n");
        }
        else
            return -1;
    }
    //printf("0 %d\n1 %d\n2 %d\n3 %d\n4 %d\n5 %d",count0,count1,count2,count3,count4,count5);
    if(count0!=count1||count2!=count3||count5>count4){
        return -1;
    }
    //if count!=0 and clear out everything
    if(getchar()==EOF)
        return 0;
    else
        return -1;
}

void clearNameBuf(){
    char* name_ptr = name_buf;
    for(int i=0;i<NAME_MAX;i++){
        *name_ptr = '\0';
        name_ptr++;
    }
    return;
}

//-1 if error else 0 leave getchar to the next header
//if type, do something
int processHeader(){
    int ch = getchar();
    if(ch!=MAGIC0)// 0x0c
        return -1;
    ch = getchar();
    if(ch!=MAGIC1)
        return -1;
    ch = getchar();
    if(ch!=MAGIC2)
        return -1;//magic sequanencs checked
    previousType = currentType;
    currentType = getchar();
    if(currentType==EOF)
        return -1;
    //start depth
    previousDepth = currentDepth;
    currentDepth = 0;
    for(int i=6;i>=0;i-=2){
        int x = getchar();
        if(x==EOF)
            return -1;
        currentDepth+=(x*power(16,i));
    };//start size
    currentSize = 0;
    for(int i=14;i>=0;i-=2){
        int x = getchar();
        if(x==EOF)
            return -1;
        currentSize+=(x*power(16,i));
    }//type, depth, size initialized
    return 0;//if getchar starts nextline
}

int power(int a,int b){
    int output = 1;
    for(int i=0;i<b;i++){
        output*=a;
    }
    return output;
}

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 0 if validation succeeds and -1 if validation fails.
 * Upon successful return, the selected program options will be set in the
 * global variable "global_options", where they will be accessible
 * elsewhere in the program.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 0 if validation succeeds and -1 if validation fails.
 * Refer to the homework document for the effects of this function on
 * global variables.
 * @modifies global variable "global_options" to contain a bitmap representing
 * the selected options.
 */
int validargs(int argc, char **argv) //check for path
{
    if (argc<=1)
        return -1;
    if(equalstring(*(argv+1),"-h")==0){// if 1st flag is -h
        global_options = 0x1;
        return 0; // success
    }
    if(equalstring(*(argv+1),"-s")==0){// if 1st flag is -s
        if(argc==2){
            global_options = 0x2;
            //return serialize(); // success
            return 0;
        }
        if(argc==3||argc>4||equalstring(*(argv+2),"-p")!=0){
            return -1;
        }
        //bin -s -p dir
        if(validDir(*(argv+3))==0){
            global_options = 0x2;
            path_init(*(argv+3));
            return 0;
           // return serialize();
        }
        return -1;
    } // -s end
    if(equalstring(*(argv+1),"-d")==0){// if 1st flag is -d
       if(argc==2){
            global_options = 0x4; //0100
            return 0;
            //return deserialize();
        }
        if(argc>5||(equalstring(*(argv+2),"-p")!=0&&equalstring(*(argv+2),"-c")!=0))
            return -1;
        if(equalstring(*(argv+2),"-c")==0){//if clobber
            if(argc>3){ // check -p dir
                if(argc==4||equalstring(*(argv+3),"-p")!=0)// bin -d -c -p
                    return -1;
                if(validDir(*(argv+4))==0){
                    global_options = 0xc; //1100
                    path_init(*(argv+4));
                    return 0;
                    //return deserialize();
                }
                return -1;
            }
            global_options = 0xc;//1100
            return 0;
            //return deserialize();
        }
        if(equalstring(*(argv+2),"-p")==0){ //bin -d -p
            if(argc==3||argc>5)
                return -1;
            if(validDir(*(argv+3))==-1)
                return -1;
            path_init(*(argv+3));
            if(argc==5&&equalstring(*(argv+4),"-c")==0){
                global_options = 0xc; //0100
                return 0;
                //return deserialize();
            }
            global_options = 0x4; //0100
            return 0;
            //return deserialize();
        }
    }
    return -1;
}

//return 0 if equal, else return -1
int equalstring(char* x, char* y){
    char* xp = x;
    char* yp = y;
    while(*xp==*yp){
        if(*xp=='\0'||*yp=='\0')
            break;
        xp++;
        yp++;
    }
    if(*xp!=*yp)
        return -1;
    else
        return 0;
}

int validDir(char* x){ //need to fix
    if(*x!='\0')
        return 0;
    if(*x=='/'&&*(x+1)=='/') // //
        return -1;
    if(*x=='/'&&*(x+1)!='/')// /d
        return 0;
    if(*x=='.'&&*(x+1)=='.'&&*(x+2)=='/'&&*(x+3)!='/')// ../t
        return 0;
    return -1;
}
