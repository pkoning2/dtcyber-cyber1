/*
 *	Master.c - Read PLATO master file.
 *
 *	Richard W. Mincher.			89/05/12.
 *
 *
 *	master mfile -l				#list master file catalog
 *		-or-
 *	master mfile -f lesson		#extract lesson source
 *		-or-
 *	master mfile -r cyl blk len	#dump raw file
 *
 *	mfile	-	path of master file.
 *	lesson	-	lesson name
 *	cyl		-	cylinder
 *	blk		-	block
 *	len		-	length
 *
 */
#if 0
#include	<Types.h>
#include 	<fcntl.h>
#include	<ErrNo.h>
#include 	<stdio.h>
#include	<ErrMgr.h>
#include	<CursorCtl.h>
#include	<Errors.h>
#else
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define true 1
#define false 0
#endif

int	masterfile = 0;

unsigned char	plato63[65] =
	"¢abcdefghijklmnopqrstuvwxyz0123456789+-*/()$= ,.è[]%•Ñ¢¢¢¢<>¢¢¢;";
unsigned char	plato70[65] =
	"¢ABCDEFGHIJKLMNOPQRSTUVWXYZ¢¢¢¢¢¢_'¢¢ãå¢?¢¢¢¢¢\"!é¢¢¢ç¢¢¢¢¢¢¢¢¢¢:";
unsigned char	plato76[65] =
	"¢êëßí¶¢¢¢¢¢¢ìîÉùï`ñóõ°®ò^¢¢úû¢¢¢@ü¢¢¢&¢¢¢Ç#Ö¢©¢¢{}¢£¢¢¢¢¢ôö¢¢¢~";
unsigned char	plato7670[65] =
	"¢â¢†á¢¢¢¢|¢¢¢¢¢¢¢¢¢¢¢¢¢Üà¢¢¢¢¢¢¢¢¢¢¢¢¢¢¢¢¢¢¢¢¢¢¢•¢¢¢¢¢¢¢¢¢¢¢¢¢¢¢";

void
platox( unsigned char dcode[], unsigned char asc[], unsigned short length )
{
	short	i, j;
	
	for(i=j=0; i < length; i++)
	{
		if (dcode[i] == 0)
			break;
		switch( dcode[i] )
		{
			case	56:
				i++;
				asc[j++] = plato70[dcode[i]];
				break;
			case	62:
				i++;
				if (dcode[i] != 56)
					asc[j++] = plato76[dcode[i]];
				else
				{
					i++;
					asc[j++] = plato7670[dcode[i]];
				}
				break;
			default:
				asc[j++] = plato63[dcode[i]];
				break;
		}
	}
	asc[j] = 0;
}

unsigned long
makenumber( unsigned char ptr[], short length )
{
	short			i;
	unsigned long	val;
	
	val = 0;
	for(i=0; i < length; i++)
		val = (val * 0x40) + ptr[i];
	return val;
}

void
readwords( unsigned long	word, unsigned char	buffer[] )
{
	unsigned	long	fbyte;
	unsigned	long	io;
	unsigned	char	temp[15];
	short				i, j;
	
	/*	Calculate byte from word	*/
	
	fbyte = ((word & ~1) * 60) >> 3;
	io = lseek( masterfile, fbyte, 0 );
	if (io == -1)
	{
		fprintf( stderr, "lseek failed - errno = %d\n", errno );
		close( masterfile );
		exit(1);
	}
	
	io = read( masterfile, temp, 15 );
	if (io == -1)
	{
		fprintf( stderr, "read failed - errno = %d\n", errno );
		close( masterfile );
		exit(1);
	}
	
	for( i=j=0; j < 15; i+=4,j+=3 )
	{
		buffer[i+0] = (temp[j+0] >> 2) & 0x3F;
		buffer[i+1] = ((temp[j+0] << 4) & 0x30) | ((temp[j+1] >> 4) & 0x0F);
		buffer[i+2] = ((temp[j+1] << 2) & 0x3C) | ((temp[j+2] >> 6) & 0x03);
		buffer[i+3] = temp[j+2] & 0x3F;
	}
	
}

void
readword( unsigned long	word, unsigned char	buffer[] )
{
	unsigned	char	temp[20];
	
	readwords( word, temp );
	if (word & 1)
		memcpy( buffer, temp+10, 10 );
	else
		memcpy( buffer, temp, 10 );
}


void
rawdump( unsigned long cylinder, unsigned long block, unsigned long blocks )
{
	short				blockcount;
	unsigned	long	ptr;
	char				last1[10];
	char				last2[10];
	char				wb1[10];
	char				text1[12];
	char				wb2[10];
	char				text2[12];
	short				i;
	short				bcount;
	char				reptmode;
	char				reptflag;
	
	text1[11] = 0;
	text2[11] = 0;
		
	ptr = (cylinder * 91 + block) * 320;
	for(blockcount=0; blockcount < blocks; blockcount++)
	{
		for(i=0; i<10; i++)
			last1[i] = last2[i] = 0xff;
		reptmode = 0;
		if (blocks != 1)
			printf("\n\n§§§§§Block %d\n", blockcount );
		for( bcount=0; bcount < 320; bcount+=2,ptr+=2 )
		{
			readword( ptr, wb1 ); 
			readword( ptr + 1, wb2 ); 
			reptflag = true;
			for(i=0; i < 10; i++)
				if ((wb1[i] != last1[i]) || (wb2[i] != last2[i]))
					reptflag = false;
			if (reptflag && (bcount != 318))
			{
				if (!reptmode)
					printf("...Repeat...\n");
				reptmode = true;
				continue;
			}
			
			reptmode = false;
			for(i=0; i<10; i++)
				text1[i] = plato63[wb1[i]];
			printf("%03.3o: %02.2o%02.2o%02.2o%02.2o%02.2o%02.2o%02.2o%02.2o%02.2o%02.2o",
				bcount,
				wb1[0], wb1[1], wb1[2], wb1[3], wb1[4], wb1[5], wb1[6], wb1[7], wb1[8], wb1[9] );

			for(i=0; i<10; i++)
				text2[i] = plato63[wb2[i]];
			printf("    %02.2o%02.2o%02.2o%02.2o%02.2o%02.2o%02.2o%02.2o%02.2o%02.2o   %10.10s  %10.10s\n",
				wb2[0], wb2[1], wb2[2], wb2[3], wb2[4], wb2[5], wb2[6], wb2[7], wb2[8], wb2[9],
				text1, text2 );
			memcpy( last1, wb1, 10 );
			memcpy( last2, wb2, 10 );
		}
	}
}

void
printinfo( unsigned char name[], unsigned char info[] )
{
	char	aname[11];
	char	tname[10];
	char	bflag;
	short	station;
	short	length;
	short	cylinder;
	short	block;
	int part, poff;
    
	platox( name, aname, 10 );
	bflag = (info[0] & 0x20) ? 'B' : ' ';
	station = ((info[0] << 6) & 0x1F) | info[1];
	switch( info[4] )
	{
		case 1:
			strcpy( tname, "LESSON" );
			break;			
		case 2:
			strcpy( tname, "BINARY" );
			break;			
		case 3:
			strcpy( tname, "COURSE" );
			break;			
		case 4:
			strcpy( tname, "STUDENT" );
			break;			
		case 5:
			strcpy( tname, "COMPASS" );
			break;			
		case 6:
			strcpy( tname, "SQUOZE" );
			break;			
		case 7:
			strcpy( tname, "DATASET" );
			break;			
		case 8:
			strcpy( tname, "PNOTES" );
			break;			
		case 9:
			strcpy( tname, "GNOTES" );
			break;			
		case 10:
			strcpy( tname, "TNOTES" );
			break;			
		case 63:
			strcpy( tname, "DIRECTORY" );
			break;			
		default:
			strcpy( "UNKNOWN", tname );
			break;
	}
	
	length = info[5];
	
	cylinder = ((info[6] << 6) & 0xFC0) | info[7];
	
	block = ((info[8] << 6) & 0xFC0) | info[9];
	
    part = cylinder * 91 + block;
    poff = part % 7;
    part /= 7;
    
	printf("%-10.10s      %c      %04.4d      %-9.9s      %03.3d      %04.4x      %4.4x %04o %d\n",
		aname, bflag, station, tname, length, cylinder, block, part, poff );
	
}

void
catalogfile()
{
	unsigned	char	wb1[20], wb2[20];
	unsigned	char	assemble[50];
	unsigned	long	number1, number2;
	unsigned	long	nameptr;
	unsigned	long	infoptr;
	short				filecount;
	short				i;

	readword( 0, wb1 );
	
	platox( wb1, assemble, 10 );
	
	printf( "Catalog of Master File %s\n", assemble );
	
	readword( 2, wb1 );
	
	/*	Calculate space	*/
	
	number1 = makenumber( &wb1[4], 3 );

	number2 = makenumber( &wb1[7], 3 );

	printf("Maximum space = %05.5ld; Space Allocated = %05.5ld\n", number1, number2 );
	
	/*	Calculate files	*/

	readword( 3, wb1 );
	
	number1 = makenumber( &wb1[4], 3 );

	filecount = makenumber( &wb1[7], 3 );

	printf("Maximum files = %05.5ld; Files Allocated = %05.5ld\n", number1, filecount );
	
	/*	Read name table pointer	*/
	
	readword( 4, wb1 );	
	
	nameptr = makenumber( &wb1[7], 3 );
	
	infoptr = nameptr + number1;
	
	printf("Name            B      Station   Type           Length   Cylinder  Block\n");
	printf("----------      -      -------   ---------      ------   --------  -----\n");

	for(i=0; i < filecount; i++ )
	{
		readword( nameptr, wb1 );
		readword( infoptr, wb2 );
				
		printinfo( wb1, wb2 );

		nameptr++;
		infoptr++;	
	}
}

void
dumpcode( unsigned long start, unsigned long finish )
{
	char	buffer[100];
	char	assem[100];
	char	*bptr;
	
	while (start < finish)
	{
		bptr = buffer;
		do
		{
			readword( start++, bptr );
			bptr += 10;
		}
		while (*(bptr-2));
		platox( buffer, assem, 90 );
		printf("%s\n", assem );
	}
}


void
printfile( unsigned long where )
{
	unsigned	char	name[12];
	unsigned	char	buffer[100];
	unsigned	char	assemble[100];
	unsigned	long	fileptr;
	unsigned	long	number;
	short				maxblock;
	short				blocks;
	short				i;
	unsigned	long	length;
	short				blocknum;
	
	fileptr = where * 320;
	
	readword( fileptr + 0, buffer );
	platox( buffer, name, 10 );
	
	readword( fileptr + 2, buffer );
	maxblock = makenumber( &buffer[8], 2 );
	
	readword( fileptr + 3, buffer );
	blocks = makenumber( &buffer[8], 2 );
	
	printf("§§§§§Lesson name - %-10.10s      Number of Blocks - %2.2d             Last block used - %2.2d\n",
		name, maxblock, blocks );
	
	readword( fileptr + 2*maxblock + 5, buffer );
	platox( buffer, assemble, 10 );
	printf("§§§§§Date Started - %-10.10s", assemble );
	readword( fileptr + 2*maxblock + 6, buffer );
	platox( buffer, assemble, 10 );
	printf("     Date last edited - %-10.10s", assemble );
	readword( fileptr + 2*maxblock + 28, buffer );
	platox( buffer, assemble, 10 );
	printf("     Time last edited - %-10.10s\n", assemble );

	readword( fileptr + 2*maxblock + 29, buffer );
	platox( buffer, assemble, 10 );
	printf("§§§§§Last editor name - %-10.10s", assemble );
	readword( fileptr + 2*maxblock + 31, buffer );
	platox( buffer, assemble, 10 );
	printf("     Last editor course - %-10.10s\n", assemble );
	
	readword( fileptr + 2*maxblock + 7, buffer );
	platox( buffer, assemble, 10 );
	printf("§§§§§Change Code - %-10.10s", assemble );
	readword( fileptr + 2*maxblock + 8, buffer );
	platox( buffer, assemble, 10 );
	printf("      Inspect Code - %-10.10s\n", assemble );
	
	readword( fileptr + 2*maxblock + 9, buffer );
	readword( fileptr + 2*maxblock + 10, buffer + 10 );
	readword( fileptr + 2*maxblock + 11, buffer + 20 );
	platox( buffer, assemble, 30 );
	printf("§§§§§Author name - %s\n", assemble );
	
	readword( fileptr + 2*maxblock + 12, buffer );
	readword( fileptr + 2*maxblock + 13, buffer + 10 );
	platox( buffer, assemble, 20 );
	printf("§§§§§Department - %-10.10s", assemble );
	
	readword( fileptr + 2*maxblock + 14, buffer );
	readword( fileptr + 2*maxblock + 15, buffer + 10 );
	platox( buffer, assemble, 20 );
	printf("       Phone number - %-10.10s\n", assemble );
	
	readword( fileptr + 2*maxblock + 16, buffer );
	readword( fileptr + 2*maxblock + 17, buffer + 10 );
	platox( buffer, assemble, 20 );
	printf("§§§§§Subject - %-20.20s", assemble );

	readword( fileptr + 2*maxblock + 18, buffer );
	readword( fileptr + 2*maxblock + 19, buffer + 10 );
	platox( buffer, assemble, 20 );
	printf("     Audience - %-20.20s\n", assemble );
	
	readword( fileptr + 2*maxblock + 20, buffer );
	readword( fileptr + 2*maxblock + 21, buffer + 10 );
	readword( fileptr + 2*maxblock + 22, buffer + 20 );
	readword( fileptr + 2*maxblock + 23, buffer + 30 );
	readword( fileptr + 2*maxblock + 24, buffer + 40 );
	readword( fileptr + 2*maxblock + 25, buffer + 50 );
	readword( fileptr + 2*maxblock + 26, buffer + 60 );
	readword( fileptr + 2*maxblock + 27, buffer + 70 );
	platox( buffer, assemble, 80 );
	printf("§§§§§Short description - %s\n", assemble );
	
	readword( fileptr + 2*maxblock + 32, buffer );
	platox( buffer, assemble, 10 );
	printf("§§§§§-COMMON- access code  - %-10.10s\n", assemble );
	readword( fileptr + 2*maxblock + 33, buffer );
	platox( buffer, assemble, 10 );
	printf("§§§§§-USE- access code     - %-10.10s\n", assemble );
	readword( fileptr + 2*maxblock + 34, buffer );
	platox( buffer, assemble, 10 );
	printf("§§§§§-JUMPOUT- access code - %-10.10s\n", assemble );
		
	for(i=0; i < blocks; i++)
	{
		readword( fileptr + 5 + maxblock + i, buffer );	/*	Get block name	*/
		platox( buffer, name, 10 );
		readword( fileptr + 5 + i, buffer );	/*	Block info		*/
		length = (makenumber( buffer + 7, 3 ) >> 9) & 0x1FF;
		blocknum = makenumber (buffer + 8, 2 ) & 0x1FF;
		printf( "\n\n§§§§§ Block %03.3d     Name - %-10.10s     Length - %03.3d     Type - ",
			blocknum, name, length );
		switch( buffer[0] & 0x1f )
		{
			case	0:
				printf("SOURCE\n");
				readword( fileptr + 4, buffer );
				if (blocknum == 0)
					dumpcode( fileptr + makenumber( buffer + 7, 3 ),  
						fileptr + makenumber( buffer + 7, 3 ) + length );
				else
					dumpcode( fileptr + (blocknum * 320), fileptr + (blocknum * 320) + length );
				break;

			case	1:
				printf("COMMON\n");
				rawdump( (where + blocknum) / 91, (where + blocknum) % 91, 1 );
				break;

			case	2:
				printf("CHARSET\n");
				rawdump( (where + blocknum) / 91, (where + blocknum) % 91, 1 );
				break;

			case	3:
				printf("MICRO\n");
				rawdump( (where + blocknum) / 91, (where + blocknum) % 91, 1 );
				break;

			case	4:
				printf("LESLIST\n");
				rawdump( (where + blocknum) / 91, (where + blocknum) % 91, 1 );
				break;

			case	5:
				printf("VOCAB\n");
				rawdump( (where + blocknum) / 91, (where + blocknum) % 91, 1 );
				break;

			case	6:
				printf("LINESET\n");
				rawdump( (where + blocknum) / 91, (where + blocknum) % 91, 1 );
				break;

			default:
				printf("UNKNOWN\n");
				rawdump( (where + blocknum) / 91, (where + blocknum) % 91, 1 );
				break;		
		}
	}
}

void
extractfile( unsigned char *name )
{
	unsigned	char	wb1[20], wb2[20];
	unsigned	char	assemble[50];
	unsigned	long	number1, number2;
	unsigned	long	nameptr;
	unsigned	long	infoptr;
	short				filecount;
	short				i;
	char				*iptr;

	readword( 3, wb1 );
	
	/*	Calculate space	*/
	
	number1 = makenumber( &wb1[4], 3 );

	filecount = makenumber( &wb1[7], 3 );
	
	readword( 4, wb1 );	
	
	nameptr = makenumber( &wb1[7], 3 );
	
	infoptr = nameptr + number1;
	
	for(i=0; i < filecount; i++ )
	{
		readword( nameptr, wb1 );
		readword( infoptr, wb2 );
				
		platox( wb1, assemble, 10 );
		if (strcmp( assemble, name ) == 0)
			break;
		nameptr++;
		infoptr++;	
	}

	if (i == filecount)
	{
		fprintf( stderr, "Couldn't find file - %s\n", name );
		exit(1);
	}
	
	printfile( makenumber( &wb2[6], 2 ) * 91 + makenumber( &wb2[8], 2 ) );
}

int
main(int argc, char *argv[])
{
	short	i;
		
	masterfile = open( argv[1], O_RDONLY );
	if (masterfile == -1)
	{
		fprintf( stderr, "Couldn't open master file - %s\n", argv[1] );
		exit(1);
	}
	
	for(i=2; i<argc; i++)
	{
		if (strcmp( argv[i], "-l" ) == 0)
		{
			catalogfile();
			close( masterfile );
			exit(0);
		}
		if (strcmp( argv[i], "-f" ) == 0)
		{
			extractfile( argv[i+1] );
			close( masterfile );
			exit(0);
		}
		if (strcmp( argv[i], "-r" ) == 0)
		{
			rawdump( atoi(argv[i+1]), atoi(argv[i+2]), atoi(argv[i+3]) );
			close( masterfile );
			exit(0);
		}
	}
		
	fprintf( stderr, "No processing options specified.\n");
	close( masterfile );
	exit(1);
}
