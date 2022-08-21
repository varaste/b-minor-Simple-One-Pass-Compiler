#include <stdio.h> 
#include <string.h> 

int main() { 
	int inputstr [100];
	int i=0;
	int j=0;
	
	int cap=0;
	int small=0;
	int aeoui=0;
	int num=0;
	
	for(i; i<100; i++){
		printf("vared kon");
		scanf(inputstr [i]);
	}
	
	for(j; j<100; j++){
		if(inputstr [j]=='A' || inputstr [j]=='B' || inputstr [j]=='C' ||
		inputstr [j]=='D' || inputstr [j]=='E' || inputstr [j]=='F' ||
		inputstr [j]=='G' || inputstr [j]=='H' || inputstr [j]=='I' ||
		inputstr [j]=='J' || inputstr [j]=='K' || inputstr [j]=='L' ||
		inputstr [j]=='M' || inputstr [j]=='N' || inputstr [j]=='O' ||
		inputstr [j]=='P' || inputstr [j]=='Q' || inputstr [j]=='R' ||
		inputstr [j]=='S' || inputstr [j]=='T' || inputstr [j]=='U'||
		inputstr [j]=='V' || inputstr [j]=='W' || inputstr [j]=='X' ||
		inputstr [j]=='Y' || inputstr [j]=='Z' ){
			cap++;
		}
		if(inputstr [j]=='a' || inputstr [j]=='b' || inputstr [j]='c' ||
		inputstr [j]=='d' || inputstr [j]=='e' || inputstr [j]=='f' ||
		inputstr [j]=='g' || inputstr [j]=='h' || inputstr [j]=='i' ||
		inputstr [j]=='j' || inputstr [j]=='k' || inputstr [j]=='lL' ||
		inputstr [j]=='m' || inputstr [j]=='n' || inputstr [j]=='o' ||
		inputstr [j]=='p' || inputstr [j]=='q' || inputstr [j]=='r' ||
		inputstr [j]=='s' || inputstr [j]=='t' || inputstr [j]=='u'||
		inputstr [j]=='v' || inputstr [j]=='w' || inputstr [j]=='x' ||
		inputstr [j]=='y' || inputstr [j]=='z' ){
			small++;
		}
		if(inputstr [j]=='a' || inputstr [j]=='e' || inputstr [j]=='u' ||
		inputstr [j]=='i' || inputstr [j]=='o'){
			aeoui++;
		}
		if(inputstr [j]=='1' || inputstr [j]=='2' || inputstr [j]=='3' ||
		inputstr [j]=='4' || inputstr [j]=='5' || inputstr [j]=='6' ||
		inputstr [j]=='7' || inputstr [j]=='8' || inputstr [j]=='9' ||
		inputstr [j]=='0'){
			num++;
		}
	}
	
	printf("Tedad Bozorg: ")
	printf("%d",cap);
	printf("\nTedad small: ")
	printf("%d",small);	
	printf("\nTedad aeoui: ")
	printf("%d",aeoui);
	printf("\nTedad num: ")
	printf("%d",num);
    return 0; 
}

