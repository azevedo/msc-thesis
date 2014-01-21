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

#ifndef __TTranspose__
#define __TTranspose__

#include "exports.h"
#include "TScoreVisitor.h"

namespace MusicXML 
{

/*!
\brief used for transposing instruments.

	If the part is being encoded for a transposing instrument
	in written vs. concert pitch, the transposition must be
	encoded in the transpose element. The transposition is
	represented by chromatic steps (required) and three
	optional elements: diatonic pitch steps, octave changes,
	and doubling an octave down. The chromatic and
	octave-change elements are numeric values added to the
	encoded pitch data to create the sounding pitch. The
	diatonic element is also numeric and allows for correct
	spelling of enharmonic transpositions.
\n
    The MusicXML \e transpose element is defined in attributes.dtd.
*/
class EXP TTranspose : public visitable, public smartable {
    
    public:
        enum { undefined = -1 };

        EXP friend SMARTP<TTranspose> newTranspose();

        virtual void accept(TScoreVisitor& visitor);

        void	setDiatonic(long diat);
        void	setChromatic(long chrom);
        void	setOctaveChge(long oct);
        void	setDouble (long dbl);

        long	getDiatonic() const		{ return fDiatonic;}
        long	getChromatic() const	{ return fChromatic;}
        long	getOctaveChge() const	{ return fOctaveChge;}
        long	getDouble() const		{ return fDouble;}

    protected:
        TTranspose() : fDiatonic(undefined), fChromatic(0), fOctaveChge(undefined), fDouble(undefined) {}
        virtual ~TTranspose() {}

    private:
        long	fDiatonic;
        long	fChromatic;
        long	fOctaveChge;
        long	fDouble;
};
typedef SMARTP<TTranspose> STranspose;

}

#endif

