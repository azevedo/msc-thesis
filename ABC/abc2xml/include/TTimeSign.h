/*
  Copyright © Grame 2003

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

    Grame Research Laboratory, 9, rue du Garet 69001 Lyon - France
    grame@grame.fr

*/

#ifndef __TTimeSign__
#define __TTimeSign__

#include "exports.h"
#include "bimap.h"
#include "TRational.h"
#include "TScoreVisitor.h"
#include <string>

namespace MusicXML 
{

/*!
\brief Represents time signatures.

	Time signatures are represented by two elements. The
	beats element indicates the number of beats, as found in
	the numerator of a time signature. The beat-type element
	indicates the beat unit, as found in the denominator of
	a time signature. The symbol attribute is used to
	indicate another notation beyond a fraction: the common
	and cut time symbols, as well as a single number with an
	implied denominator. Normal (a fraction) is the implied
	symbol type if none is specified. Multiple pairs of
	beat and beat-type elements are used for composite
	time signatures with multiple denominators, such as
	2/4 + 3/8. A composite such as 3+2/8 requires only one
	beat/beat-type pair. A senza-misura element explicitly
	indicates that no time signature is present.
\n
    The MusicXML \e time element is defined in attributes.dtd.
*/
class EXP TTimeSign : public visitable, public smartable {
    
    public:
        enum { undefined = -1, groupbeats = -1, common=1, cut, single_number, normal, last=normal };

        EXP friend SMARTP<TTimeSign> newTimeSign();

        virtual void accept(TScoreVisitor& visitor);

		void	setSymbol(int symbol);
		int		getSymbol() const			{ return fSymbol; }

        //! beat type must be \c groupbeat for the first elements of a composite like 3+2/8
        long    add (long beat, long type);
        //! senza measures are implicitely denoted by an empty beat list
		bool	measured ()	const			{ return fBeat.size() > 0; }
        //! returns the time signature as a rational number (senza measure is -1/1)
		TRational rational();
        
        vector<long>& getBeats()		{ return fBeat; }
        vector<long>& getBeatTypes()	{ return fBeatType; }

	//! convert an integer symbol to a MusicXML string
	static const string	xmlsymbol (int d);
	//! convert an MusicXML string to an integer symbol
	static int			xmlsymbol (const string str);

    protected:
        //! empty constructor denotes unmeasured time
        TTimeSign() : fSymbol(undefined) {}
        virtual ~TTimeSign() {}

    private:
        vector<long> fBeat;
        vector<long> fBeatType;
        int 		fSymbol;

	static bimap<string, int> fSymbol2String;
	static int 		fSymbolTbl[];
	static string 	fSymbolStrings[];
};
typedef SMARTP<TTimeSign> STimeSign;

} // namespace MusicXML


#endif
