/*

  MusicXML Library
  Copyright (C) 2003  Grame

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  Grame Research Laboratory, 9 rue du Garet, 69001 Lyon - France
  grame@grame.fr

*/

#ifndef __TScorePartwise__
#define __TScorePartwise__

#include "exports.h"
#include "TMeasure.h"
#include "TPart.h"
#include "TScore.h"
#include "TScoreHeader.h"
#include "TScoreVisitor.h"
#include "smartpointer.h"

namespace MusicXML 
{

/*!
\brief The MusicXML partwise \e measure element.
*/
class EXP TPWMeasure : public TMeasure {

    public:
		enum { allVoices=-1 };
        EXP friend SMARTP<TPWMeasure> newPWMeasure(string num);
        EXP friend SMARTP<TPWMeasure> newPWMeasure(long num);
        virtual void accept(TScoreVisitor& visitor);

        //! adds music data and returns its index;
        long add (const SMusicData& data);
        vvector<SMusicData>& data()				{ return fData; }
		unsigned long getDuration(long voice=allVoices);

    protected:
        TPWMeasure(string num);
        TPWMeasure(long num);
        virtual ~TPWMeasure();
    private:
        vvector<SMusicData> 	fData;          
};
typedef SMARTP<TPWMeasure> SPWMeasure;

/*!
\brief The MusicXML partwise \e part element.
*/
class EXP TPWPart : public TPart {
    public:
        EXP friend SMARTP<TPWPart> newPWPart(string id);
        virtual void accept(TScoreVisitor& visitor);

        //!! adds a measure and returns its index;
        long add (const SPWMeasure& measure);
        vvector<SPWMeasure>& measures()			{ return fMeasures; }

    protected:
        TPWPart(const string& id);
        virtual ~TPWPart();
    private:
        vvector<SPWMeasure> fMeasures;

};
typedef SMARTP<TPWPart> SPWPart;

/*!
\brief a MusicXML partwise score.
*/
class EXP TScorePartwise : public TScore {
    public:
		EXP friend SMARTP<TScorePartwise> newScorePartwise();

        //!! adds a part and returns  its index;
		long add(const SPWPart& part);
        vvector<SPWPart>&  	partList() 			{ return fPartList; }

        virtual void accept(TScoreVisitor& visitor);

    protected:
        TScorePartwise();
        virtual ~TScorePartwise();
    private:
        vvector<SPWPart> fPartList;
};
typedef SMARTP<TScorePartwise> SScorePartwise;

}

#endif
