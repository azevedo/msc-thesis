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

#ifndef __TMidiDevice__
#define __TMidiDevice__

#include "exports.h"
#include "smartpointer.h"
#include "TScoreVisitor.h"
#include <string>

namespace MusicXML 
{

/*!
\brief Corresponds to the DeviceName meta event in Standard MIDI Files. 
	
	The optional port attribute is a number from 1 to 16 that can be used
	with the unofficial MIDI port (or cable) meta event.
	It correspond to the MusicXML \e midi-device element as
	defined in score.dtd.
*/
class EXP TMidiDevice : public visitable, public smartable {
    
    public:
		enum { undefined=-1 };
        EXP friend SMARTP<TMidiDevice> newMidiDevice();
        
        virtual void accept(TScoreVisitor& visitor);

        void 	setPort(long port);
		void	setName(const std::string& name);
        long 	getPort() const					{ return fPort; }
		const std::string&	getName() const 	{ return fName; }

    protected:
        TMidiDevice() : fPort(undefined) {}
        virtual ~TMidiDevice() {}
    private:
        //! the optionnal port attribute
        long 	fPort;
        //! the device name
		std::string	fName;
};
typedef SMARTP<TMidiDevice> SMidiDevice;

} // namespace MusicXML


#endif
