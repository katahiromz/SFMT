#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>

#include "shortbase128.h"
#include "util.h"
#include "sfmt-st.h"
#include "sfmt-util.h"

NTL_CLIENT;

//mat_GF2 debug_mat0;
//mat_GF2 debug_mat;
//bool debug_dependent[128];
//uint32_t debug_count;
//int debug_flag = 0;

//static void get_next_state(in_status *st);
//void dprintnext(in_status *st);
//void dprintbase(char *file, int line, int num, in_status *st);
static uint32_t get_shortest(bool dependents[], int count[], int last_mode[]);
static bool get_dependent_trans(bool dependent[], vec_GF2 array[]);
static bool dependent_rows(bool result[], mat_GF2& mat);
static void convert(mat_GF2& mat, vec_GF2 array[], uint32_t bit_len);
//static void change_norm_mode(in_status bases[], vec_GF2 next[]);
//static uint32_t last_process(in_status bases[], vec_GF2 next[]);

static sfmt_t bases[128 + 1];

static int bit_len;
static int status_mode;
static int max_weight_mode;

int get_equiv_distrib32(int bit, sfmt_t *sfmt) {
    static sfmt_t sfmtnew;
    int dist, min;
    uint32_t mode;

    min = INT_MAX;
    //printf("dist = ");
    for (mode = 0; mode < 4; mode++) {
	sfmtnew = *sfmt;
	set_up(32, bit, mode);
	dist = get_shortest_base(&sfmtnew);
	//printf("%d ", dist);
	//printf("dist = %d\n", dist);
	if (dist < min) {
	    min = dist;
	}
    }
    //printf("\n");
    return min;
}

int get_equiv_distrib64(int bit, sfmt_t *sfmt) {
    static sfmt_t sfmtnew;
    int dist, min;
    uint32_t mode;

    min = INT_MAX;
    //printf("dist = ");
    for (mode = 0; mode < 4; mode += 2) {
	sfmtnew = *sfmt;
	set_up(64, bit, mode);
	dist = get_shortest_base(&sfmtnew);
	//printf("%d ", dist);
	//printf("dist = %d\n", dist);
	if (dist < min) {
	    min = dist;
	}
    }
    //printf("\n");
    return min;
}

int get_equiv_distrib128(int bit, sfmt_t *sfmt) {
    static sfmt_t sfmtnew;
    int dist;

    sfmtnew = *sfmt;
    set_up(128, bit, 0);
    dist = get_shortest_base(&sfmtnew);
    return dist;
}

void set_up(uint32_t bit_mode, uint32_t len, uint32_t p_mode) {
    switch (bit_mode) {
    case 32:
	max_weight_mode = 4;
	if ((len <= 0) || (len > 32)) {
	    printf("bitLength error mode 32\n");
	    exit(1);
	}
	break;
    case 64:
	max_weight_mode = 2;
	break;
    case 128:
    default:
	max_weight_mode = 1;
	break;
    }
    bit_len = len * max_weight_mode;
    status_mode = p_mode;
}
  
int get_shortest_base(sfmt_t *sfmt) {
    vec_GF2 next[bit_len + 1];
    vec_GF2 debug_next;
    int count[bit_len + 1];
    int last_mode[bit_len + 1];
    bool dependents[bit_len + 1];
    int shortest, min_mode;
    int i, j;
    bool dependent_found;
    int weight_mode = max_weight_mode;
    int debug_count;

    for (i = 0; i < bit_len; i++) {
	set_special(&(bases[i]), i);
    }
    for (i = 0; i <= bit_len; i++) {
	last_mode[i] = weight_mode;
    }
    bases[bit_len] = *sfmt;
    bases[bit_len].special = false;
    for (;;) {
	for (i = 0; i <= bit_len; i++) {
	    count[i] = get_vector(next[i], &bases[i], status_mode,
				  weight_mode, bit_len, max_weight_mode);
	}
	dependent_found = get_dependent_trans(dependents, next);
#if 0
	if (weight_mode < max_weight_mode) {
	    printf("dependent_foud:%d\n", dependent_found);
	    cout << "dependent:";
	    for (i = 0; i <= bit_len; i++) {
		cout << dependents[i];
	    }
	    cout << endl;
	    for (i = 0; i <= bit_len; i++) {
		cout << next[i] << " " << count[i] << " " 
		     << last_mode[i]<< endl;
	    }
	}
#endif
	while ((!dependent_found) && (weight_mode > 1)) {
	    weight_mode--;
#if 0
	    for (i = 0; i <= bit_len; i++) {
		cout << next[i] << " " << count[i] << " "
		     << last_mode[i]<< endl;
	    }
#endif
	    for (i = 0; i <= bit_len; i++) {
		count[i] = get_vector(next[i], &bases[i], status_mode,
				      weight_mode, bit_len, max_weight_mode);
		if (last_mode[i] == max_weight_mode) {
		    for (j = (bit_len * weight_mode / max_weight_mode);
			 j < bit_len; j++) {
			if (!IsZero(next[i][j])) {
			    last_mode[i] = weight_mode;
			    break;
			}
		    }
		}
	    }
	    dependent_found = get_dependent_trans(dependents, next);
#if 0
	    printf("dependent_foud:%d\n", dependent_found);
	    cout << "dependent:";
	    for (i = 0; i <= bit_len; i++) {
		cout << dependents[i];
	    }
	    cout << endl;
	    for (i = 0; i <= bit_len; i++) {
		cout << next[i] << " " << count[i] << " "
		     << last_mode[i]<< endl;
	    }
#endif
	}
	if (!dependent_found) {
	    break;
	}
	shortest = get_shortest(dependents, count, last_mode);
#if 0
	for (i = 0; i <= bit_len; i++) {
	    cout << dependents[i];
	}
	cout << endl;
	for (i = 0; i <= bit_len; i++) {
	    cout << next[i] << " " << count[i] << endl;
	}
	cout << "before add " << next[shortest] << " " << count[shortest] 
	     << " " << bases[shortest].special << endl;
#endif
	debug_count = count[shortest];
	debug_next = next[shortest];
	if (debug_count > 0) {
	    debug_count = 1;
	}
	for (i = 0; i <= bit_len; i++) {
	    if (i == shortest) {
		continue;
	    }
	    if (dependents[i]) {
		add_rnd(&(bases[shortest]), &(bases[i]),
			count[i] - count[shortest]);
		last_mode[shortest] = weight_mode;
		debug_next += next[i];
	    }
	}
	if (!IsZero(debug_next)) {
	    printf("next is not zero!\n");
	}
#if 0
	count[0] = get_vector32(next[0], &bases[shortest], status_mode,
				weight_mode, bit_len);
	cout << "count " << debug_count << " -> " << count[0] << endl;
	//cout << "after add " << next[0] << " " << count[0] << " " 
	//     << bases[shortest].special << endl;
#endif
    }
#if 0
    for (i = 0; i <= bit_len; i++) {
	if (count[i] == INT_MAX) {
	    printf("%d:zero\n", i);
	} else {
	    printf("%d:mode(%d):count(%d) = %d\n", i, last_mode[i],
		   count[i], count[i] * max_weight_mode 
		   - (max_weight_mode - last_mode[i]));
	}
    }
#endif
    shortest = INT_MAX;
    for (i = 0; i <= bit_len; i++) {
	if (count[i] < shortest) {
	    shortest = count[i];
	}
    }
#if 0
    for (i = 0; i <= bit_len; i++) {
	//if (count[i] == shortest) {
	if (count[i] != INT_MAX) {
	    printf("%d:mode(%d):count(%d) = %d\n", i, last_mode[i],
		   count[i], 
		   count[i] * max_weight_mode 
		   - (max_weight_mode - last_mode[i])) ;
	} else {
	    printf("%d:mode(%d):zero\n", i, last_mode[i]);
	}
    }
#endif
    min_mode = 5;
    for (i = 0; i <= bit_len; i++) {
	if (count[i] == shortest) {
	    if (last_mode[i] < min_mode) {
		min_mode = last_mode[i];
	    }
	}
    }
    return shortest * max_weight_mode - (max_weight_mode - min_mode);
}

static uint32_t get_shortest(bool dependents[], int count[], int last_mode[]) {
    int index = 0;
    int min = INT_MAX;
    int i;

    for (i = 0; i <= bit_len; i++) {
	if (dependents[i]) {
	    if (last_mode[i] == max_weight_mode && count[i] < min) {
		min = count[i];
		index = i;
	    }
	}
    }
    if (min == INT_MAX) {
	printf("can't find shortest\n");
	exit(1);
    }
    return index;
}

static bool get_dependent_trans(bool dependent[], vec_GF2 array[]) {
    mat_GF2 mat;
    uint32_t rank;

    convert(mat, array, bit_len);
    //debug_mat0 = mat;
    rank = (uint32_t) gauss_plus(mat);
    return dependent_rows(dependent, mat);
}

static bool dependent_rows(bool result[], mat_GF2& mat) {
    int32_t rows;
    int32_t cols;
    int i, j, k, pos;
    bool found = false;

    rows = mat.NumRows();
    cols = mat.NumCols();
    for (i = 0; i < cols; i++) {
	result[i] = false;
    }
    pos = 0;
    for (i = 0; (i < cols - pos) && (i < rows); i++) {
	if (IsOne(mat.get(i, i + pos))) {
	    continue;
	}
	for (j = 0; j < i; j++) {
	    if (IsOne(mat.get(j, i + pos))) {
		for (k = 0; k < i + pos; k++) {
		    if (IsOne(mat.get(j, k))) {
			found = true;
			result[k] = true;
			break;
		    }
		}
	    }
	}
	if (found) {
	    result[i + pos] = true;
	    break;
	} else {
	    pos++;
	    i--;
	}
    }
    if (!found) {
#if 0
	cout << "not found" << endl;
	cout << mat << endl;
	exit(1);
#endif
    }
    //debug_mat = mat;
    return found;
}

static void convert(mat_GF2& mat, vec_GF2 array[], uint32_t size) {
    uint32_t i, j;

    mat.SetDims(size + 1, size + 1);
    for (i = 0; i < size + 1; i++) {
	for (j = 0; j < size; j++) {
	    mat.put(j, i, array[i].get(j));
	}
    }
    for (i = 0; i < size + 1; i++) {
	mat.put(size, i, 0);
    }
}