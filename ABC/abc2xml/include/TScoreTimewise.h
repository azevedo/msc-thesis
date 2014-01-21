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

#ifndef __TScoreTimewise__
#define __TScoreTimewise__

#include "exports.h"
#include "TMeasure.h"
#include "TPart.h"
#include "TScore.h"
#include "TScoreHeader.h"
#include "smartpointer.h"

namespace MusicXML 
{

/*!
\brief The MusicXML timewise \e part element.
*/
class EXP TTWPart : public TPart {
    public:
        EXP friend SMARTP<TTWPart> newTWPart(string id);
        virtual void accept(TScoreVisitor& visitor);

        long add (const SMusicData& data);
		vvector<SMusicData>& data()					{ return fData; }

    protected:
        TTWPart(string id) : TPart(id) {}
        virtual ~TTWPart() {}
    private:
        vvector<SMusicData> fData;

};
typedef SMARTP<TTWPart> STWPart;

/*!
\brief The MusicXML timewise \e measure element.
*/
class EXP TTWMeasure : public TMeasure {

    public:
        EXP friend SMARTP<TTWMeasure> newTWMeasure(string num);
        EXP friend SMARTP<TTWMeasure> newTWMeasure(long num);

        virtual void accept(TScoreVisitor& visitor);

        long add (const SMARTP<TTWPart>& part);
        vvector< SMARTP<TTWPart> >& parts()			{ return fParts; }

    protected:
        TTWMeasure(string num) : TMeasure(num) {}
        TTWMeasure(long num) : TMeasure(num) {}
        virtual ~TTWMeasure() {}
    private:
        vvector< SMARTP<TTWPart> > 	fParts;          
};
typedef SMARTP<TTWMeasure> STWMeasure;

/*!
\brief a MusicXML timewise score.
*/
class EXP TScoreTimewise : public TScore {
    public:
		EXP friend SMARTP<TScoreTimewise> newScoreTimewise();
        virtual void accept(TScoreVisitor& visitor);

        long 	add(const STWMeasure& part);
        vvector<STWMeasure>&  measureList() 		{ return fMeasureList; }

    protected:
        TScoreTimewise();
        virtual ~TScoreTimewise() {}
    private:
        vvector<STWMeasure> fMeasureList;
};
typedef SMARTP<TScoreTimewise> SScoreTimewise;

}

#endif
