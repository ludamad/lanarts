/*
 * featuregen.h
 *
 *  Created on: Mar 19, 2012
 *      Author: 100397561
 */

#ifndef FEATUREGEN_H_
#define FEATUREGEN_H_

#include "mtwist.h"
#include "GeneratedLevel.h"

struct FeatureGenSettings {
	int nstairs_up, nstairs_down;
	FeatureGenSettings(int nups, int ndowns) :
			nstairs_up(nups), nstairs_down(ndowns) {
	}
};

void generate_features(const FeatureGenSettings& rs, MTwist& mt, GeneratedLevel& level);


#endif /* FEATUREGEN_H_ */
