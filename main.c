# Copyright (c) 2016-.. #John
#
# Author: #John <pocolab.com@gmail.com>
# URL: http://www.pocolab.com
# Version: 1.0.0

# Commentary:

# Sudoku solver can solve any sudoku. 

# License:

# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 3
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Emacs; see the file COPYING.  If not, write to the
# Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
# Boston, MA 02110-1301, USA.


#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <curses.h>
#include <time.h>

typedef struct {
	size_t number; 
	size_t minimap; 
	size_t *possible_numbers;
}number;

typedef struct{
	number **number_indexes; // indexes
}minimap;

typedef struct{
	size_t id;
	ssize_t weight;
}possible_number_weight;

typedef struct{
	number *numbers;
	size_t size; // [0-z]
	minimap *minimaps;
	size_t current_possible_number;
	possible_number_weight *possible_numbers_order;
	size_t *recommend_possible_numbers;
}map;

typedef struct{
	char *filename_data;
	char *filename_map;

	char *data_str;
	char *map_str;
} init_data;

void print_help();

char* read_init_data_file(char *filename);
void read_init_data(init_data *data);

char *init_str(char *str);
map *init_map(char *data_str, char *map_str);
void validate_map(map*map);
void remove_impossible_numbers(map*map);
size_t set_all_possible_numbers(map*m);

size_t is_row_contains(map *map, size_t num,size_t row);
size_t is_column_contains(map * map, size_t num,size_t column);
size_t is_minimap_contains(map * map, size_t num,size_t minimap);
size_t is_any_contains(map *map,size_t n,size_t row,size_t column,size_t minimap);
void solve(map *map);
map *alloc_map(size_t size);
map *makecopy_of_map(map *map);
size_t get_solved_numbers(map *map);
void find_and_set_number_from_possibles(map *map, number *n,size_t row, size_t column);
void reinit_map_numbers(map *m, map *mc);
size_t check_is_number_possible(map **mc_arr,size_t index, size_t pn);
void check_m_map(map **mc_arr,size_t index);
size_t get_map_possible_numbers_count(map *m);
void print_possible_numbers_count(map *m);
	
void print(map *map);

///////////////////////////////////////////////////////////////////////////////
// Helpers
char*read_file(char *filename);
	
int main(int argc, char *argv[]){
	if(argc!=3){
		print_help();
		exit (EXIT_FAILURE);	
	}
	init_data data=(init_data){argv[1],argv[2]};
	read_init_data(&data);	
	map *map=init_map(data.data_str,data.map_str);
	validate_map(map);
	solve(map);
	print(map);
	printf("hell\n");
	return(0);
}
size_t is_row_contains(map *map, size_t num,size_t row){
	size_t i;
	number *n;
	for(i=0;i<map->size;i++){
		n=&map->numbers[row*map->size+i];	
		if(n->number==num){
			return 1;
		}
	}
	return 0;
}
size_t is_column_contains(map * map, size_t num,size_t column){
	size_t i;
	number *n;
	for(i=0;i<map->size;i++){
		n=&map->numbers[i*map->size+column];	
		if(n->number==num){
			return 1;
		}
	}
	return 0;
}
size_t is_minimap_contains(map * map, size_t num,size_t minimap){
	size_t i;
	number *n;
	number **nn=map->minimaps[minimap].number_indexes;
	for(i=0;i<map->size;i++){
		n=(number *)nn[i];	
		if(n->number==num){
			return 1;
		}
	}
	return 0;
}
size_t is_any_contains(map *map,size_t n,size_t row,size_t column,size_t minimap){
	return is_row_contains(map,n,row)||
		is_column_contains(map,n,column)||
		is_minimap_contains(map,n,minimap);
}

size_t get_solved_numbers(map *map){
	size_t i;
	size_t len=map->size*map->size;
	size_t c=0;
	number *n;
	for(i=0;i<len;i++){
		n=&map->numbers[i];
		if (n->number>0) {
			c++;
		}
	}
	return c;
}
size_t get_not_solved_numbers_count(map *map){
	size_t i;
	size_t len=map->size*map->size;
	size_t c=0;
	number *n;
	for(i=0;i<len;i++){
		n=&map->numbers[i];
		if (n->number==0) {
			c++;
		}
	}
	return c;
}
size_t get_possible_numbers_count(size_t *pn, size_t size){
	size_t i,c=0;
	for (i=0;i<size; i++) {
		if(pn[i]>0){c++;}
	}
	return c;
}
size_t is_map_solved(map *m,size_t check_zero){
	size_t i,j,k;
	size_t id;
	size_t size=m->size;
	for(i=0;i<size;i++){
		for(j=0;j<size;j++){
			id=i*size+j;
			if(!m->numbers[id].number){
				if(check_zero){
					return 0;
				}
			}else{
				for(k=j+1;k<size;k++){
					if(m->numbers[i*size+k].number==m->numbers[id].number){
						return 0;
					}
				}
			}
		}
	}
	return 1;
}

void print_map(map *map){
	//system("clear");
	size_t i,j,v;
	for (i = 0; i < map->size; i++) {
		for (j=0; j<map->size; j++) {
			v=map->numbers[i*map->size+j].number;
			if(v>9){
				v+=7+32;
			}
			printf("%c ",(int)v+'0');
		}
		printf("\n");
		
	}
	printf("\n");
		
}
void print_map_with_grid(map *map){
	//system("clear");
	size_t i,j,v;
	size_t size=map->size;
	for(i=0;i<size-1;i++){
		if(i==0){
			printf("+-");
		}
		printf("----");
		if(i==size-2){
			printf("--+\n");
		}
	}
	for (i = 0; i < map->size; i++) {
		for (j=0; j<map->size; j++) {
			if(j==0){
				printf("| ");
			}
			v=map->numbers[i*map->size+j].number;
			if(v>9){
				v+=7+32;
			}
			printf("%c ",(int)v+'0');

			if(j<size-1){
				if(map->numbers[i*map->size+j+1].minimap!=
				   map->numbers[i*map->size+j].minimap){
					printf("| ");
				}else{
					printf("  ");
				}
			}else if (j==size-1){
				printf("|\n");
			}
		}

		if(i<size-1){
		for(j=0;j<map->size;j++){
			if(j==0){
				printf("|");
			}
			if(j<size-1){
				if(map->numbers[i*map->size+j].minimap!=
				   map->numbers[(i+1)*map->size+j].minimap){
					printf("---");
				}else{
										printf("   ");
				}
				if((map->numbers[i*map->size+j].minimap!=
				    map->numbers[(i+1)*map->size+j].minimap)
				   ||
				(map->numbers[i*map->size+j+1].minimap!=
				    map->numbers[(i+1)*map->size+j+1].minimap)
				   ){
					printf("+");
				}
				else{
if((map->numbers[i*map->size+j+1].minimap==
					    map->numbers[(i+1)*map->size+j+1].minimap)
					   &&
					   (map->numbers[(i+1)*map->size+j].minimap!=
					    map->numbers[(i+1)*map->size+j+1].minimap)
					   
					   ){
						printf("|");
					}
					else{
						printf(" ");
					}

				}
			}else if (j==size-1){
				if(map->numbers[i*map->size+j].minimap!=
				   map->numbers[(i+1)*map->size+j].minimap){
					printf("---|\n");
				}else{
					printf("   |\n");
				}
			}
		}
		}
		
	}
for(i=0;i<size-1;i++){
		if(i==0){
			printf("+-");
		}
		printf("----");
		if(i==size-2){
			printf("--+\n");
		}
	}
		
}

void print_map_with_grid_with_possible_numbers(map *map){
	//system("clear");
	size_t i,j,v,k,l,c,r,d;
	size_t size=map->size;
	c=ceil(sqrt(size));
	r=c;
	for(i=0;i<size-1;i++){
		if(i==0){
			printf("+-");
		}
		printf("---");
			for(k=0;k<c+1;k++){printf("-");}
		if(i==size-2){
			for(k=0;k<c+1;k++){printf("-");}
			printf("-+\n");
		}
	}
	for (i = 0; i < map->size; i++) {
			
		for(l=0;l<r;l++){
		printf("|  ");
			for (j=0; j<map->size; j++) {
				if (map->numbers[i*size+j].number==0) {
					d=l*c+r;
					for(k=l*c;k<d;k++){
						if(k<size){
							v=map->numbers[i*size+j].possible_numbers[k];
							if (v>0) {
								if(v>9){
									v+=7+32;
								}
								printf("%c",(int)v+'0');
							}else{
								printf("-");}
						}
						else {
							printf(" ");		
						}
					}
				}
				else{
					for(k=0;k<c;k++){
						if(l==r/2-1&&k==c/2-1){
							v=map->numbers[i*size+j].number;
							if(v>9){
								v+=7+32;
							}
							printf("%c",(int)v+'0');
						}else{printf(" ");}}
				}
				if(j==size-1){printf(" |\n");}else{


					if(j<size-1){
						if(map->numbers[i*map->size+j+1].minimap!=
						   map->numbers[i*map->size+j].minimap){
							printf(" |  ");
						}else{
							printf("    ");
						}
					}



					//printf("   ");

				}
			}
		}

		if(i<size-1){
		for(j=0;j<map->size;j++){
			if(j==0){
				printf("|");
			}
			if(j<size-1){
				if(map->numbers[i*map->size+j].minimap!=
				   map->numbers[(i+1)*map->size+j].minimap){
					printf("--");
			for(k=0;k<c+1;k++){printf("-");}
				}else{
										printf("  ");
			for(k=0;k<c+1;k++){printf(" ");}
				}
				if((map->numbers[i*map->size+j].minimap!=
				    map->numbers[(i+1)*map->size+j].minimap)
				   ||
				(map->numbers[i*map->size+j+1].minimap!=
				    map->numbers[(i+1)*map->size+j+1].minimap)
				   ){
					printf("+");
				}
				else{
if((map->numbers[i*map->size+j+1].minimap==
					    map->numbers[(i+1)*map->size+j+1].minimap)
					   &&
					   (map->numbers[(i+1)*map->size+j].minimap!=
					    map->numbers[(i+1)*map->size+j+1].minimap)
					   
					   ){
						printf("|");
					}
					else{
						printf(" ");
					}

				}
			}else if (j==size-1){
				if(map->numbers[i*map->size+j].minimap!=
				   map->numbers[(i+1)*map->size+j].minimap){
			for(k=0;k<c+1;k++){printf("-");}
					printf("--|\n");
				}else{
			for(k=0;k<c+1;k++){printf(" ");}
					printf("  |\n");
				}
			}
		}
		}
		
	}
for(i=0;i<size-1;i++){
		if(i==0){
			printf("+-");
		}
		printf("---");
			for(k=0;k<c+1;k++){printf("-");}
		if(i==size-2){
			for(k=0;k<c+1;k++){printf("-");}
			printf("-+\n");
		}
	}
		
}



int compare_possible_number_weights(const void * a,
				    const void * b){
	possible_number_weight *oa = (possible_number_weight *)a;
	possible_number_weight *ob = (possible_number_weight *)b;
	return ( ob->weight-oa->weight );
}
//void set_best_possible_numbers_order(map *m,size_t id){
//size_t size=m->size;
//size_t len=size*size;
//number *n;
//possible_number_weight *arr=calloc(size,sizeof(possible_number_weight));
//size_t i,k,j,r;
//for(i=0;i<len;i++){
//n=&m->numbers[i];
//if(n->number>0){
//arr[n->number-1].id=n->number;
//arr[n->number-1].weight++;
//}
//}
//	
//size_t pnc;
//size_t pn;
//for(j=0;j<size;j++){
//pn=possible_numbers[j];
//if(pn>0){
//
//for(r=0;r<size;r++){
//if(m->recommend_possible_numbers[r]>0){
//if(pn==
//m->recommend_possible_numbers[r])
//{
//arr[j].weight=size*4+3*(size-r);//r c m w
//goto cont;
//}
//}
//else {
//break;
//}
//}
//	
//pnc=0;
//for(i=0;i<size;i++){
//n=&m->numbers[i*size+row];
//if(n->number==0){
//for(k=0;k<size;k++){
//if (n->possible_numbers[k]==pn) {
//pnc++;
//break;
//	
//}
//}
//}
//n=&m->numbers[column*size+i];
//if(n->number==0){
//for(k=0;k<size;k++){
//if (n->possible_numbers[k]==pn) {
//pnc++;
//break;
//	
//}
//}
//}
//n=(number *)m->minimaps[minimap].number_indexes[i];
//if(n->number==0){
//for(k=0;k<size;k++){
//if (n->possible_numbers[k]==pn) {
//pnc++;
//break;
//	
//}
//}
//}
//}
//arr[j].weight+=3*size-pnc;
//cont:;
//}
//}
// 
//qsort (arr, size, sizeof(possible_number_weight), compare_possible_number_weights);
//for(i=0;i<size;i++){
//possible_numbers[i]=arr[i].id;
//}
//free(arr);
//arr=0;
//}

void set_possible_numbers_weight(map *m){
	size_t size=m->size;
	size_t len=size*size;
	number *n;
	size_t id;
	size_t i,j,k,r;
	possible_number_weight *arr=calloc(size*size,sizeof(possible_number_weight));
	size_t *arr_minimap=calloc(size,sizeof(size_t));
	size_t *arr_row=calloc(size,sizeof(size_t));
	size_t *arr_column=calloc(size,sizeof(size_t));
	size_t pnc;
	for(i=0;i<size;i++){
		for(j=0;j<size;j++){
			id=i*size+j;
			n=&m->numbers[id];
			if(n->number>0){
				arr_minimap[n->minimap]++;
				arr_row[i]++;
				arr_column[j]++;
				arr[id].id=id;
				arr[id].weight=-1;
			}
			else {
				pnc=0;
				for(k=0;k<size;k++){
					if(n->possible_numbers[k]>0){
						pnc++;
						arr[id].id=id;
					}
				}
				for(k=0;k<size;k++){
					arr[id].weight=size*(size-pnc);
					for(r=0;r<size;r++){
						if(m->recommend_possible_numbers[r]>0){
							if(n->possible_numbers[k]==
							   m->recommend_possible_numbers[r])
								{
									arr[id].weight=size*4+3*(size-r);//r c m w
								}
						}
						else {
							break;
						}
					}
				}

			}
			
		}
	}
	for(i=0;i<size;i++){
		for(j=0;j<size;j++){
			id=i*size+j;

			n=&m->numbers[id];
			if(n->number==0){
				if (arr[id].weight==size-1||
				    arr_row[i]==size-1||
				    arr_column[j]==size-1||
				    arr_minimap[n->minimap]==size-1
				    ) {
					arr[id].weight+=size*size; //r c m w r_p_n
				}
				else{
					arr[id].weight+=
						arr_row[i]+arr_column[j]+arr_minimap[n->minimap];
				}
			}
		}
	}
	free(arr_minimap);
	free(arr_row);
	free(arr_column);

	qsort (arr, len, sizeof(possible_number_weight), compare_possible_number_weights);

	m->possible_numbers_order=arr;
	//for(i=0;i<len;i++){
	//printf("%d:%d  ",arr[i].id,arr[i].weight);
	//}
	//printf("\n");
	//exit(0);
}
ssize_t set_recomend_possible_numbers(map *m,size_t *possible_numbers){
	size_t i,j;
	size_t pn;
	for(i=0;i<m->size;i++){
		if(m->recommend_possible_numbers[i]>0){
			for(j=0;j<m->size;j++){
				pn=possible_numbers[j];
				if(m->recommend_possible_numbers[i]==pn){
					return pn;
				}
			}
		}
		else{
			for(j=0;j<m->size;j++){
				pn=possible_numbers[j];
				m->recommend_possible_numbers[i]=pn;
				return pn;
			}
		}
		return -1;
	}
}
ssize_t set_recomend_possible_number(map *m,size_t possible_number){
	size_t i;
	for(i=0;i<m->size;i++){
		if(m->recommend_possible_numbers[i]>0){
			return possible_number;
		}
		else{
			m->recommend_possible_numbers[i]=possible_number;
			return possible_number;
		}
		return -1;
	}
}

size_t check_map2(map **mc_arr,size_t index){
		
	size_t size=mc_arr[0]->size;
	size_t i,k;
	number *n;
	size_t pn;
	size_t id;

	size_t frame_index;
	size_t len=size*size;
	size_t r_p_n;


	remove_impossible_numbers(mc_arr[index]);
	//print_map_with_grid_with_possible_numbers(mc_arr[index]);
	//	printf("cool\n");

	while(set_all_possible_numbers(mc_arr[index])){
		//	print_map_with_grid_with_possible_numbers(mc_arr[index]);
		//printf("fuck1\n");
		remove_impossible_numbers(mc_arr[index]);
		//print_map_with_grid_with_possible_numbers(mc_arr[index]);
		//printf("fuck2\n");
	}

	
	if (index<size/2) {
		//printf("index:%d\n",index);
		print_map_with_grid_with_possible_numbers(mc_arr[index]);
		//printf("fuck\n");
		//print_map_with_grid(mc_arr[index]);
	}
	//if (index>0) {	exit(0);	}


	if(!is_map_solved(mc_arr[index],0)){
		return 0;
	}


	for(i=0;i<len;i++){
		if(mc_arr[index]->possible_numbers_order[i].weight>-1){
			id=mc_arr[index]->possible_numbers_order[i].id;
			n=&mc_arr[index]->numbers[id];
			if (n->number==0) {
				
				//set_best_possible_numbers_order(mc_arr[index],id);
				for(k=0;k<size;k++){
					pn=n->possible_numbers[k];

					if(pn>0){
						reinit_map_numbers(mc_arr[index],mc_arr[index+1]);
						mc_arr[index+1]->current_possible_number=pn;
							
						if(is_any_contains(mc_arr[index+1],pn,id/size,id%size,n->minimap)){
							n->possible_numbers[k]=0;
							continue;
						}
						mc_arr[index+1]->numbers[id].number=pn;
						//printf("pn:%d\n",pn);
						//printf("r:%d;c:%d\n",id/size,id%size);


						r_p_n=set_recomend_possible_number(mc_arr[index+1],pn);

						free(mc_arr[index+1]->possible_numbers_order);
						mc_arr[index+1]->possible_numbers_order=0;
						set_possible_numbers_weight(mc_arr[index+1]);
						

						frame_index=check_map2(mc_arr,index+1);
						if(frame_index>0){
							return frame_index;
							n->number=pn;
							// TODO: check this
							continue;
						}
						else {
						       
							n->possible_numbers[k]=0;
						}
					}
				}
				if (n->number==0) {
					return 0;	
				}
				else{
					free(mc_arr[index]->numbers[id].possible_numbers);
					mc_arr[index]->numbers[id].possible_numbers=0;
				}
					
			}
		}
	}

	if(!is_map_solved(mc_arr[index],1)){
	return 0;
	}
	return index;

}






size_t check_map(map **mc_arr,size_t index){
		
	size_t size=mc_arr[0]->size;
	size_t i,j,k;
	number *n;
	size_t pn;
	size_t id;

	size_t c=0;
	size_t frame_index;

	if(index < size*size/5.2){
		printf("index: %d\n",index);
		print_map(mc_arr[index]);
	}
	
	for(i=0;i<size;i++){
		for(j=0;j<size;j++){
			id=i*size+j;
			n=&mc_arr[index]->numbers[id];
			if (n->number==0) {
				
				if(get_possible_numbers_count((size_t *)n->possible_numbers,size)==0){
					return 0;	
				}
			
				
				for(k=0;k<size;k++){
					pn=n->possible_numbers[k];
					

					if(pn>0){
						reinit_map_numbers(mc_arr[index],mc_arr[index+1]);
						mc_arr[index+1]->current_possible_number=pn;

						
						if(is_any_contains(mc_arr[index+1],pn,i,j,n->minimap)){

							n->possible_numbers[k]=0;
							continue;
						}
						mc_arr[index+1]->numbers[id].number=pn;




						//
						// Solving one by one 
						//
						
						//for(p=1;p<index+2;p++){
						//	reinit_map_numbers(mc_arr[index+1],mc_arr[index+2]);
						//if(!check_is_number_possible(mc_arr,index+2,mc_arr[p+1]->current_possible_number))
						//		{
						//			printf("fucked");
						//			n->possible_numbers[k]=0;
						//			goto cont;
						//		}
						//else{
						 
						//	}
						//}





						
						
						
						frame_index=check_map(mc_arr,index+1);
						if(frame_index>0){
							print(mc_arr[frame_index]);
							return frame_index;
						}
						if(frame_index>0)
							{
								printf("fuck %d\n",index);
								n->number=pn;
								// TODO: check this
								continue;
							}
						else {
						       
							n->possible_numbers[k]=0;
						}
					}
				cont:;
				}
				if (n->number==0) {
					return 0;	
				}
				else{
					// Just to save some memory
					free(mc_arr[index]->numbers[id].possible_numbers);
					mc_arr[index]->numbers[id].possible_numbers=0;
				}
					
			}
			else{
				c++;
			}
		}
	}
	
	if(c==size*size)
		{
			return index;
		}
	return 0;

}


size_t check_is_number_possible(map **mc_arr,size_t index, size_t pn){
		
	size_t size=mc_arr[0]->size;
	size_t i,j;
	number *n;
	size_t id;
	size_t c;


	for(i=0;i<size;i++){

		for(j=0;j<size;j++){
			id=i*size+j;
			n=&mc_arr[index]->numbers[id];
			if(n->number==pn){
				break;
			}
			if (n->number==0) {
				if (n->possible_numbers[pn-1]==pn) {
					reinit_map_numbers(mc_arr[index],mc_arr[index+1]);
					mc_arr[index+1]->numbers[id].number=pn;
					if(check_is_number_possible(mc_arr,index+1,pn)){
						n->number=pn;
					}
					else{
						n->possible_numbers[pn-1]=0;
					}

				}
				
			}
		}
	}
	for(i=0;i<size;i++){
		c=0;
		for(j=0;j<size;j++){
			if (n->number==pn) {
				c=1;
				break;
			}
		}
		if(c==0){
			return 0;
		}
	}

	return 1;
}




void check_m_map(map **mc_arr, size_t index){
	size_t i,j,k;
	size_t id;
	size_t size=mc_arr[0]->size;
	size_t pn;
	number *n;
	for(i=0;i<size;i++){
		for(j=0;j<size;j++){
			id=i*size+j;
			n=&mc_arr[index]->numbers[id];
			if(n->number==0){
				for (k=0; k<size;k++) {
					pn=n->possible_numbers[k];
					if (pn>0) {
						reinit_map_numbers(mc_arr[index],mc_arr[index+1]);
						if(0==check_is_number_possible(mc_arr,index+1,pn))
							{
								n->possible_numbers[k]=0;
							}
						else{

						}
						
					}
				}

			}
		}
	}
}

size_t get_map_possible_numbers_count(map *m){
	
	size_t i,c=0;
	for (i = 0; i<m->size*m->size; i++) {
		if(m->numbers[i].number==0){
			c+=get_possible_numbers_count(m->numbers[i].possible_numbers,m->size);
		}
	}
	return c;
}
void print_possible_numbers_count(map *m){
	
	size_t c=get_map_possible_numbers_count(m);
	printf("possible_numbers_count: %d\n",(int)c);
}


void solve(map *m){
	remove_impossible_numbers(m);
	while(set_all_possible_numbers(m)){
	remove_impossible_numbers(m);
	}
	m->recommend_possible_numbers=calloc(m->size,sizeof(size_t));
	set_possible_numbers_weight(m);
	
	size_t i;
	size_t len_maps =//m->size*m->size+
		1+	get_not_solved_numbers_count(m);
	map **mc_arr=malloc(len_maps*sizeof(map *));
	
	for (i=0; i<len_maps; i++) {
		mc_arr[i]=makecopy_of_map(m);
	}

	size_t index = check_map2(mc_arr,0);
	if (index==-1) {
		fprintf(stderr,"Smth goig wrong, check input data.");
		exit (EXIT_FAILURE);
	}
	*m=*mc_arr[index];

	printf("--\n");
	print_map(mc_arr[index]);
	exit(1);
	// Do not free memory, just print and exit
}

map *alloc_map(size_t size){
	size_t len=size*size;
	map *m=malloc(sizeof(map));
	m->size=size;
	m->numbers=calloc(len,sizeof(number));
	
	size_t j;
	minimap *mm;
	m->minimaps=malloc(m->size*sizeof(minimap));
	for(j=0;j<m->size;j++){
		mm=&m->minimaps[j];
		mm->number_indexes=calloc(m->size,sizeof(number *));
	}
		
	return m;
}
void reinit_map_numbers(map *m, map *mc){
	size_t i=-1;
	number *n,*nc;
	size_t len=m->size*m->size;
	while(++i<len){
		n=&m->numbers[i];
		nc=&mc->numbers[i];
		if(n->number==0){
			if(!nc->possible_numbers){
				nc->possible_numbers=malloc(m->size*sizeof(m->size));
			}
			memcpy(nc->possible_numbers,n->possible_numbers,m->size*sizeof(size_t));
		}
		else if (nc->possible_numbers){
			free(nc->possible_numbers);
			nc->possible_numbers=0;
		}
		nc->number=n->number;
		nc->minimap=n->minimap;
	}
}
map *makecopy_of_map(map *m){
	map *mc=alloc_map(m->size);
	mc->recommend_possible_numbers=malloc(m->size*sizeof(size_t));
	memcpy(mc->recommend_possible_numbers,m->recommend_possible_numbers,
	       m->size*sizeof(size_t));
	mc->possible_numbers_order=malloc(m->size*sizeof(size_t));
	memcpy(mc->possible_numbers_order,m->possible_numbers_order,
	       m->size*sizeof(size_t));
	
	size_t i=-1;
	number *n,*nc;
	size_t len=m->size*m->size;
	size_t *minimap_arr=calloc(m->size,sizeof(size_t));
	minimap *mm;
	while(++i<len){
		n=&m->numbers[i];
		nc=&mc->numbers[i];
		nc->number=n->number;
		nc->minimap=n->minimap;
		mm=&mc->minimaps[nc->minimap];
		mm->number_indexes[minimap_arr[nc->minimap]]=(number *)&nc->number;
		minimap_arr[n->minimap]++;
		if (n->number==0) {
			nc->possible_numbers=malloc(m->size*sizeof(m->size));
			memcpy(nc->possible_numbers,n->possible_numbers,m->size*sizeof(size_t));
		}
	}

	free(minimap_arr);
	
	return mc;
}
void find_and_set_number_from_possibles(map *map, number *n,size_t row, size_t column){
	size_t k;
	size_t counter;
	size_t pn;
	size_t pn_save;
	if(n->number==0){
		counter=0;
		for(k=0;k<map->size;k++){
			pn=n->possible_numbers[k];
			if(pn>0){
				if(is_any_contains(map,pn,row,column,n->minimap)){
					n->possible_numbers[k]=0;
				}
				else{
					counter++;
					pn_save=pn;
				}
			}
		}
		if (counter==1) {
			n->number=pn_save;
			free(n->possible_numbers);
			n->possible_numbers=0;
		}
	}	
}
void remove_impossible_numbers(map*map){

	size_t i,j;
	number *n;
	for (i=0;i<map->size;i++){
		for (j=0;j<map->size;j++) {
			n=&map->numbers[i*map->size+j];	
			find_and_set_number_from_possibles(map, n,i,j);
		}
	}
}
ssize_t find_number_id(map *m,number *n){
	size_t i;
	for(i=0;i<m->size*m->size;i++){
		if(&m->numbers[i]==n){
			return i;
		}
	}
	return -1;
}

size_t set_all_possible_numbers(map*m){

	size_t i,j,k,id;
	size_t size=m->size;
	number *nr,*nc,*nm;
	size_t *pr,*pc,*pm;
	size_t *prn,*pcn,*pmn;
	size_t flag=0;
	pr=malloc(size*sizeof(size_t));
	pc=malloc(size*sizeof(size_t));
	pm=malloc(size*sizeof(size_t));

	prn=malloc(size*sizeof(size_t));
	pcn=malloc(size*sizeof(size_t));
	pmn=malloc(size*sizeof(size_t));
	
	for (i=0;i<size;i++){
		memset(pr,0,size*sizeof(size_t));
		memset(pc,0,size*sizeof(size_t));
		memset(pm,0,size*sizeof(size_t));
		for (j=0;j<size;j++) {			
			nc=&m->numbers[i*size+j];
			nr=&m->numbers[j*size+i];
			nm=(number *)&m->minimaps[i].number_indexes[j]->number;
			for(k=0;k<size;k++){
				if(!nc->number&&nc->possible_numbers[k]>0){
					pc[k]++;
					pcn[k]=i*size+j;
				}
				if(!nr->number&&nr->possible_numbers[k]>0){
					pr[k]++;
					prn[k]=j*size+i;
					//printf("%d %d,%d %d\n",i,j,nr->possible_numbers[k], pr[k]);
				}
				if(!nm->number&&nm->possible_numbers[k]>0){
					pm[k]++;
					pmn[k]=j;
				}
			}
		}
		//printf("new lin===\n");
		for(k=0;k<size;k++){
			if (pc[k]==1) {
				nc=&m->numbers[pcn[k]];
				if(nc->number==0&&
				   !is_any_contains(m,k+1,pcn[k]/size,pcn[k]%size,nc->minimap)){
					nc->number=k+1;
					//printf("%d;%dc\n",nc->number,pcn[k]);
					free(nc->possible_numbers);
					nc->possible_numbers=0;
					flag=1;
				}
			}
			if (pr[k]==1) {
				nr=&m->numbers[prn[k]];
				if(nr->number==0&&
				   !is_any_contains(m,k+1,prn[k]/size,prn[k]%size,nr->minimap)){
					nr->number=k+1;
					//printf("%d;%dr\n",nr->number,prn[k]);
					free(nr->possible_numbers);
					nr->possible_numbers=0;
					flag=1;
					
					//if(prn[k]==6){exit(0);}/////////TODO:
				}
			}
			if (pm[k]==1) {
				nm=(number *)&m->minimaps[i].number_indexes[pmn[k]]->number;
				if(nm->number==0){
					id=find_number_id(m,nm);
					//printf("%d\n",id);
					//exit(0);
					if(!is_any_contains(m,k+1,id/size,id%size,nm->minimap)){
						nm->number=k+1;
						//printf("%d;%dm\n",nm->number,i);
						free(nm->possible_numbers);
						nm->possible_numbers=0;
						flag=1;
					}
				}
			}
			if(flag){goto exit;}
		}

	}
 exit:
	free(pr);
	free(pc);
	free(pm);
	free(prn);
	free(pcn);
	free(pmn);
	return flag;
}

///////////////////////////////////////////////////////////////////////////////
// Init data 

char *init_str(char *str){
	size_t len=strlen(str);
	char *new_str=malloc(len*sizeof(char));
	size_t i=0;
	char *s;
	size_t l;
	while ((s=strsep(&str, ","))) {
		l = strlen(s);

		while(isspace(s[l - 1])) --l;
		while(* s && isspace(* s)) ++s, --l;
		
		if (s[0]!='\0') {
			new_str[i]=s[0];
			i++;
			
		}
	};
	free(s);
	return new_str;
}

void validate_map(map*map){
	size_t i;
	size_t l=map->size*map->size;
	size_t *arr_minimap=calloc(map->size,sizeof(size_t));
	size_t *arr_number=calloc(map->size,sizeof(size_t));
	size_t minimap_size=((1+map->size)*map->size)/2;
	number *n;
	for(i=0;i<l;i++){
		n=&map->numbers[i];
		arr_minimap[n->minimap]++;
		arr_number[n->minimap]+=n->number;
	}

	size_t v;
	size_t j;
	for(i=0;i<map->size;i++){
		if(arr_number[i]>minimap_size||arr_minimap[i]!=map->size){
			fprintf(stderr,"minimap numbers count: %d, but have to be: %d\n",
				(int)arr_minimap[i],(int)map->size);
			fprintf(stderr,"minimap numbers sum: %d, but have to be less or equal: %d\n",
				(int)arr_number[i],(int)minimap_size);
			fprintf(stderr, "Minimap %d is not valid. minimap numbers:\n",(int)i);
			
			for(j=0;j<l;j++){
				n=&map->numbers[j];
				if(n->minimap==i){
					v=n->number;
					if(v>9){
						v+=39;
					}
					fprintf(stderr,"%c ",(int)v+'0');
				}
			}
			fprintf(stderr,"\n");
			exit (EXIT_FAILURE);
		}
	}
	free(arr_number);
	free(arr_minimap);
}

map* init_map(char *data_str, char *map_str){
	char *data_str_str=init_str(data_str);
	free(data_str);
	char *map_str_str=init_str(map_str);
	free(map_str);
	
	size_t len=strlen(data_str_str);
	size_t a=sqrt(len);
	size_t b=sqrt(strlen(map_str_str));

	if (a!=(size_t)b) {
		fprintf(stderr,"Input data not valid.\n");
		fprintf(stderr,"datalen: %d; data: %s\n",(int)a,data_str_str);
		fprintf(stderr,"maplen: %d; map: %s\n",(int)b,map_str_str);
		exit (EXIT_FAILURE);	
	}

	map *m=alloc_map(a);
	
	
	size_t i=-1,j;
	number *n;
	minimap *mm;
	
	size_t *minimap_arr=calloc(m->size,sizeof(size_t));
	while(++i<len){
		n=&m->numbers[i];
		n->number=strtol((char[]){data_str_str[i], 0}, NULL, 16);
		n->minimap=strtol((char[]){map_str_str[i], 0}, NULL, 16);
		mm=&m->minimaps[n->minimap];
		mm->number_indexes[minimap_arr[n->minimap]]=(number *)&n->number;
		minimap_arr[n->minimap]++;
		if (n->number==0) {
			n->possible_numbers=malloc(m->size*sizeof(size_t));	
			for (j=0;j<m->size;j++) {
				n->possible_numbers[j]=j+1;	
			}
		}
	}
	free(data_str_str);
	free(map_str_str);
	free(minimap_arr);

	return m;
}

void print(map *map){
	return;
	initscr();
	start_color();

	number *ns=map->numbers;
	size_t s=map->size;
	size_t i,j;
	size_t l;
	number n;
	size_t v;

	size_t r1,r2,r3;

	srand ( time(NULL) );
	for(i=1;i<s+1;i++){
		r1 = 400+rand() % 400;
		r2 = 200+rand() % 400;
		r3 = 200+rand() % 400;
		init_color(i,r1,r2,r3);  
		init_color(s+i,r1,r2,r3);  
		init_pair(i, COLOR_BLACK, i);
	}
		
	for(i=0;i<s;i++){
		l=i*s;
		for(j=0;j<s;j++){
			n=ns[l+j];
			v=n.number;
			if(v>9){
				v+=7;
			}
			attron(COLOR_PAIR(n.minimap%7+1));
			printw("%c ",v+'0');
			attron(COLOR_PAIR(0));
		}
		printw("\n");
	}
	refresh();
	getch();
	endwin();
}

///////////////////////////////////////////////////////////////////////////////
// Read init data
char* read_init_data_file(char *filename){
	char *data=read_file(filename);
	if (data==NULL) {
		fprintf (stderr, "There nothing to read from: %s \n",
			 filename);
		exit (EXIT_FAILURE);	
	}
	return data;
}
void read_init_data(init_data *data){
	data->data_str=read_init_data_file(data->filename_data);
	data->map_str=read_init_data_file(data->filename_map);
}

///////////////////////////////////////////////////////////////////////////////
// Print help 
void print_help(){
	printf("\n    Usage: sudokusolver data.sudoku map.sudoku\n\n");
}

///////////////////////////////////////////////////////////////////////////////
// Helpers
char*read_file(char *filename)
{
	char *buffer = NULL;
	size_t string_size, read_size;
	FILE *handler = fopen(filename, "r");

	if (handler)
		{
			fseek(handler, 0, SEEK_END);
			string_size = ftell(handler);
			rewind(handler);

			buffer = (char*) malloc(sizeof(char) * (string_size + 1) );

			read_size = fread(buffer, sizeof(char), string_size, handler);

			buffer[string_size] = '\0';

			if (string_size != read_size)
				{
					free(buffer);
					buffer = NULL;
				}

			fclose(handler);
		}

	return buffer;
}

