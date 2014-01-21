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

#ifndef __TFiguredBass__
#define __TFiguredBass__

#include "exports.h"
#include "common.h"
#include "smartpointer.h"
#include "TScoreVisitor.h"

#include <string>

namespace MusicXML 
{

/*!
\brief figure for figured bass
*/
class EXP TFigure : public smartable, public visitable {
    public:
		EXP friend SMARTP<TFigure> newFigure();
        virtual void accept(TScoreVisitor& visitor);

		void	setPrefix(const std::string& data)		{ fPrefix = data; }
		void	setNumber(const std::string& data)		{ fNumber = data; }
		void	setSuffix(const std::string& data)		{ fSuffix = data; }
		const std::string&   getPrefix() const			{ return fPrefix; }
		const std::string&   getNumber() const			{ return fNumber; }
		const std::string&   getSuffix() const			{ return fSuffix; }
				
	protected:
        TFigure() {}
        virtual ~TFigure() {}
	private:
		string fPrefix, fNumber, fSuffix;
};
typedef SMARTP<TFigure> SFigure;

/*!
\brief figured bass

	Figured bass elements take their position from the first
	regular note that follows. We will need to clarify these
	definitions over time.
*/
class EXP TFiguredBass : public TMusicData, public Positionable, public Editorialable {

    public:
		enum { undefined=-1 };
		EXP friend SMARTP<TFiguredBass> newFiguredBass();
        virtual void accept(TScoreVisitor& visitor);

		void			setDuration (unsigned long d)   { fDuration = d; }
		unsigned long	getDuration () const			{ return fDuration; }
		long add(const SFigure& fig);
		vvector<SFigure>& figures()					{ return fFigures; }
		TPrintout&	printout()						{ return fPrintout; }

	protected:
        TFiguredBass() : fDuration(undefined) {}
        virtual ~TFiguredBass() {}
	private:
		vvector<SFigure> 	fFigures;
		long				fDuration;
		TPrintout			fPrintout;
};
typedef SMARTP<TFiguredBass> SFiguredBass;


} // namespace MusicXML


#endif
