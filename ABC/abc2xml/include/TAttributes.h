/*

  MusicXML Library
  Copyright (C) 2003-2004  Grame

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
  research@grame.fr

*/

#ifndef __TAttributes__
#define __TAttributes__

#include "exports.h"

#include "common.h"
#include "TClef.h"
#include "TKey.h"
#include "TTimeSign.h"
#include "TTranspose.h"
#include "TScoreVisitor.h"
#include "TStaffDetails.h"
#include "TMeasureStyle.h"

namespace MusicXML 
{
/*!
\brief Represents musical directives.

	Directives are like directions, but can be grouped 
	together with attributes for convenience. This is
	typically used for tempo markings at the beginning
	of a piece of music. The language is Italian ("it")
	by default.
\n	TDirective corresponds to the MusicXML \e directive element.
*/
class EXP TDirective : public Fontable, public visitable, public smartable {

    public:
        EXP friend SMARTP<TDirective> newDirective();

		virtual void accept(TScoreVisitor& visitor);

        //! the xml:lang attribute
        void setLang(const string& lang);
        //! textual directive
        void setValue(const string& val);

        const string& getLang() const			{ return fLang; }
        const string& getValue() const			{ return fValue; }

    protected:
        TDirective() {}
        virtual ~TDirective() {};
        
    private:
        std::string	fLang;
        std::string	fValue;
};
typedef SMARTP<TDirective> SDirective;

/*!
\brief
	Contains information that typically changes on measure boundaries. 
	
	It includes key and time signatures, clefs, transpositions,
	and staving. TAttributes corresponds to the MusicXML
	\e attribute element as defined in attributes.dtd.

\todo clarify the Instrument type and semantic
*/
class EXP TAttributes : public TMusicData, public Editorialable {

    public:
        enum { undefined = -1 };
        
        EXP friend SMARTP<TAttributes> newAttributes();
 
		virtual void accept(TScoreVisitor& visitor);
        
        void	setDivisions (long div);
        void	setStaves (long staves);
        void	setInstrument (long inst);
        long	getDivision () const        { return fDivision; }
        long	getStaves () const          { return fStaves; }
        long	getInstrument () const      { return fInstrument; }
        
        SKey&       key()                   { return fKey; }
        STimeSign&  timeSign()              { return fTimeSign; }
        STranspose& transpose()             { return fTranspose; }
        SDirective& directive()             { return fDirective; }
        SStaffDetails& staffDetails()       { return fStaffDetails; }
        SMeasureStyle& measureStyle()       { return fMeasureStyle; }
        vvector<SClef>& clefs()             { return fClefList; }

        long add (const SClef& clef);

    protected:
        TAttributes() : fDivision(undefined), fStaves(undefined),fInstrument(1) {}
        virtual ~TAttributes() {}
       
    private:
        long 		fDivision;
        SKey		fKey;
        STimeSign 	fTimeSign;
        long 		fStaves;
        long		fInstrument;
        vvector<SClef> fClefList;
		SStaffDetails	fStaffDetails;
        STranspose 	fTranspose;
        SDirective 	fDirective;
		SMeasureStyle fMeasureStyle;
};
typedef SMARTP<TAttributes> SAttributes;

} // namespace MusicXML


#endif
