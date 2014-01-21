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

#ifndef __TMidiInstrument__
#define __TMidiInstrument__

#include "exports.h"
#include "smartpointer.h"
#include "TScoreVisitor.h"
#include <string>

namespace MusicXML 
{
/*!
\brief Represents a MIDI instrument

	TMidiInstrument includes the necessary to address a
	specific MIDI instrument (channel, bank, program)
	+ additionnal information required by the MusicXML
	\e midi-instrument element as defined in common.dtd.
*/
class EXP TMidiInstrument : public visitable, public smartable {
    
    public:
		enum { undefined = -1 };
        EXP friend SMARTP<TMidiInstrument> newMidiInstrument(std::string id);

		//! ID is a MusicXML required attribute that refers to the score-instrument affected by the change.
 		void	setID (const std::string& id);
       //! optionnal MIDI channel numbers range from 1 to 16.
        void	setChannel (long chan);
		//! optionnal MIDI name, corresponds to ProgramName meta-events within a Standard MIDI File.
		void	setName (const std::string& name);
        //! optionnal MIDI bank number, range from 1 to 16,384.
		void	setBank (long bank);
        //! optionnal MIDI program number, range from 1 to 128.
		void	setProgram (long prog);
		/*!
			optionnal : for unpitched instruments, specify a MIDI note number
			ranging from 1 to 128. Usually used with MIDI banks for percussion.
		*/
		void	setUnpitched (long pitch)	{ fMidiUnpitched = pitch; }

 		const std::string&	getID () const		{ return fID; }
        long	getChannel () const			{ return fMidiChannel; }
		const std::string&	getName () const	{ return fMidiName; }
		long	getBank () const			{ return fMidiBank; }
		long	getProgram () const			{ return fMidiProgram; }
		long	getUnpitched () const		{ return fMidiUnpitched; }

        virtual void accept(TScoreVisitor& visitor);
		
    protected:
        TMidiInstrument(std::string ident)
                :fID(ident), fMidiChannel(undefined),fMidiBank(undefined),
                fMidiProgram(undefined),fMidiUnpitched(undefined){}
        virtual ~TMidiInstrument() {}

    private:
        std::string fID;
		long 		fMidiChannel;
        std::string fMidiName;
		long 		fMidiBank;
		long 		fMidiProgram;
        long 		fMidiUnpitched;
};	
typedef SMARTP<TMidiInstrument> SMidiInstrument;

} // namespace MusicXML


#endif
