/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/* Copyright (C) 2004 
 *
 * Authors: Erik Eliasson <eliasson@it.kth.se>
 *          Johan Bilien <jobi@via.ecp.fr>
*/


#include<samplerate.h> //include this in the installation instructions
#include <assert.h>
#include <iostream>
#include "SpAudio.h"
#include"../soundcard/SoundIO.h"
#include<libmutil/MemObject.h>


#define POS 5       //number of positions in the spatial audio scheme


/**
 * Definition of the delay values depending on the position.
 */

int32_t SpAudio::lchdelay[POS]={0,0,0,36,0};
int32_t SpAudio::rchdelay[POS]={0,36,0,0,0};

/**
 ** Definition of the matrix with the new position to be assigned depending
 ** on the number of calls being maintained
 **/

int32_t SpAudio::assmatrix[POS][POS]={{3,1,1,1,1},{0,5,5,5,5},{0,0,3,2,2},{0,0,0,4,4},{0,0,0,0,3}};


SpAudio::SpAudio(int32_t numPos){

  nPos=numPos;

}

int32_t SpAudio::spatialize(short *input,
			    MRef<SoundSource *> src,
			    short *outbuff)
{
  

  for(int32_t i=0;i<1764;i++){
    if(i%2 == 0){
      src->leftch[(src->j+lchdelay[src->position-1])%950]=input[i];
      src->j=(src->j+1)%950;
    }

    else {
      src->rightch[(src->k+rchdelay[src->position-1])%950]=input[i];
      src->k=(src->k+1)%950;
    }
  }

  for(int32_t i=0; i<882;i++){
    outbuff[(2*i)]=src->lookupleft[src->leftch[src->pointer]+32768];
    outbuff[(2*i)+1]=src->lookupright[src->rightch[src->pointer]+32768];
    src->pointer=(src->pointer+1)%950;
  }
  return src->pointer;
}


int32_t SpAudio::assignPos(int row,
			   int col)
{
  return assmatrix[row-1][col-1];
}
