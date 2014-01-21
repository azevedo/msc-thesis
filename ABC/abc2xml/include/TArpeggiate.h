/*

  MusicXML Library
  Copyright (C) 2004  Grame

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

#ifndef __TArpeggiate__
#define __TArpeggiate__

#include "exports.h"
#include "common.h"
#include "smartpointer.h"
#include "TScoreVisitor.h"

namespace MusicXML 
{

/*!
\brief MusicXML \e arpeggiate representation

	Indicates that this note is part of an arpeggiated chord.
	The number attribute can be used to distinguish between
	two simultaneous chords arpeggiated separately (different
	numbers) or together (same number). The up-down attribute
	is used if there is an arrow on the arpeggio sign. By
	default, arpeggios go from the lowest to highest note.
*/
class EXP TArpeggiate : public TNotationElement, public Positionable, public Placementable {
    public:
		enum direction { undefined, up, down };

        EXP friend SMARTP<TArpeggiate> newArpeggiate();
        virtual void accept(TScoreVisitor& visitor);
        
        void		setNumber(int n)				{ fNumber = n; }
        void		setDirection(direction dir)		{ fDirection = dir; }
        int			getNumber() const				{ return fNumber; }
        direction   getDirection() const			{ return fDirection; }
    protected:
        TArpeggiate() : fNumber(undefined), fDirection(undefined) {}
        virtual ~TArpeggiate() {}
	private:
		int			fNumber;
		direction   fDirection;
};
typedef SMARTP<TArpeggiate> SArpeggiate;


/*!
\brief MusicXML representation for \e non-arpeggiate

	Indicates that this note is at the top or bottom of
	a bracket indicating to not arpeggiate these notes.
	Since this does not involve playback, it is only
	used on the top or bottom notes, not on each note
	as for the arpeggiate element.
*/
class EXP TNonArpeggiate : public TNotationElement, public Positionable, public Placementable {
    public:
		enum type { undefined, top, bottom };

        EXP friend SMARTP<TNonArpeggiate> newNonArpeggiate();
        virtual void accept(TScoreVisitor& visitor);
        
        void 	setNumber(int n)			{ fNumber = n; }
        void 	setType(type t)				{ fType = t; }
        int		getNumber() const			{ return fNumber; }
        type	getType() const				{ return fType; }
    protected:
        TNonArpeggiate() : fNumber(undefined), fType(undefined) {}
        virtual ~TNonArpeggiate() {}
	private:
		int		fNumber;
		type	fType;
};
typedef SMARTP<TNonArpeggiate> SNonArpeggiate;


} // namespace MusicXML


#endif
