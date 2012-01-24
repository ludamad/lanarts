/*
 * mtwist.h
 *
 *  Created on: Apr 2, 2011
 *      Author: 100397561
 */

#ifndef MTWIST_H_
#define MTWIST_H_

#define N 624
class MTwist {
	unsigned long mt[N];
	int mti;

	void init_genrand(unsigned long s);
	void init_by_array(unsigned long init_key[], int key_length);

public:
	MTwist() : mti(N+1){
	}
	MTwist (unsigned long s) : mti(N+1){
		init_genrand(s);
	}
	MTwist(unsigned long init_key[], int key_length) : mti(N+1){
		init_by_array(init_key, key_length);
	}

	/* generates a random number on [0,0xffffffff]-interval */
	unsigned long genrand_int32(void);

	/* generates a random number on [0,0x7fffffff]-interval */
	long genrand_int31(void);

	/* generates a random number on [0,1]-real-interval */
	double genrand_real1(void);

	/* generates a random number on [0,1)-real-interval */
	double genrand_real2(void);

	/* generates a random number on (0,1)-real-interval */
	double genrand_real3(void);

	/* generates a random number on [0,1) with 53-bit resolution*/
	double genrand_res53(void);
};
extern MTwist twist_state;

#endif /* MTWIST_H_ */
