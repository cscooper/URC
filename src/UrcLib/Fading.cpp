/*
 *  Fading.cpp - Calculates Rayleigh or Rician fading for CORNER based on the situation
 *  Copyright (C) 2012  C. S. Cooper, A. Mukunthan
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 *  Contact Details: Cooper - andor734@gmail.com
 *  		     Mukunthan - lexxonnet@gmail.com
 */

#include "Fading.h"
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <math.h>
#include <cfloat>

using namespace std;
using namespace Urc;

DECLARE_SINGLETON(Fading);

Fading::Fading(const char* componentsFile, int seed) {
	srand(seed);
	ifstream fin;
	fin.open(componentsFile);
	
	if (fin.fail()) {
		THROW_EXCEPTION("Error opening components file: %s\n", componentsFile);
	}
	
	string buffer;
	double temp;

	while (1) {
		getline(fin, buffer);
		istringstream sfin(buffer);
		if (sfin.peek() == '#') {
			continue;
		}
		//This is really hackish. Make sure we do something a bit more fool-proof later on
		if (sfin.peek() == 'S') {
			sfin>>buffer;
			sfin>>mSamplingRate;
		} else if (sfin.peek() == 'B') {
			sfin>>buffer;
			sfin>>mBaseDopplerFrequency;
		} else if (sfin.peek() == 'N') {
			sfin>>buffer;
			sfin>>mNumGaussianComponents;
			mComponents1.reserve(mNumGaussianComponents);
			mComponents2.reserve(mNumGaussianComponents);
			break;
		}
	}
	
	for (int i=0; i<mNumGaussianComponents; i++) {
		fin>>temp;
		mComponents1.push_back(temp);
		fin>>temp;
		mComponents2.push_back(temp);
	}
	
	random_shuffle(mComponents1.begin(), mComponents1.end());
	random_shuffle(mComponents2.begin(), mComponents2.end());
	mCurrentIndex = 0;
	pthread_mutex_init(&mListMutex, NULL);
	
	
	fin.close();
}

Fading::~Fading() {
	pthread_mutex_destroy(&mListMutex);
}
	

void Fading::SetArrayIdx() {
	if (mCurrentIndex < mNumGaussianComponents) {
		//just increment the array index
		pthread_mutex_lock(&mListMutex);
		++mCurrentIndex;
		pthread_mutex_unlock(&mListMutex);
	} else {
		//reset the index to 0 and reshuffle the array
		pthread_mutex_lock(&mListMutex);
		mCurrentIndex = 0;
		random_shuffle(mComponents1.begin(), mComponents1.end());
		random_shuffle(mComponents2.begin(), mComponents2.end());
		pthread_mutex_unlock(&mListMutex);
	}
}

double Fading::CalculateFading(int classification, double kFactor) {
	if ( kFactor == DBL_MAX )
		return 1;
	if (classification == 0) {
		//rician fading for LOS		
		SetArrayIdx();
		return ((pow((mComponents1[mCurrentIndex]+sqrt(2.0*kFactor)),2)+pow(mComponents2[mCurrentIndex],2))/(2*(kFactor+1)));
	} else if (classification <= 2) {
		//rayleigh fading for NLOS1/2
		SetArrayIdx();
		return ((pow(mComponents1[mCurrentIndex],2)+pow(mComponents2[mCurrentIndex],2))/2);
	} else {
		return 0;
	}
}
