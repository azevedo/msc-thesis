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

#ifndef __TSound__
#define __TSound__

#include "exports.h"
#include "TMidiInstrument.h"
#include "common.h"
#include "conversions.h"

namespace MusicXML 
{

/*!
\brief Represents the MusicXML \e sound element.

	The sound element contains general playback parameters,
	based on MuseData's C0 sound suggestions. They can
	stand alone within a part/measure, or be a component
	element within a direction.
\n	
	Segno and dalsegno are used for backwards jumps to a
	segno sign; coda and tocoda are used for forward jumps
	to a coda sign. If there are multiple jumps, the value
	of these parameters can be used to name and distinguish
	them. If segno or coda is used, the divisions attribute
	can also be used to indicate the number of divisions
	per quarter note. Otherwise sound and MIDI generating
	programs may have to recompute this.
\n
	A dalsegno or dacapo attribute indicates that the
	jump should occur the first time through; a tocoda
	attribute indicates that the jump should occur the second
	time through.
\n
	The fine attribute follows the final note or rest in a
	movement with a da capo direction. If numeric, the value
	represents the actual duration of the final note or rest,
	which can be ambiguous in written notation and different
	among parts and voices. The value may also be "yes" to
	indicate no change to the final duration.
\n
    The MusicXML \e sound element is defined in direction.dtd.
    All its attributes are implied.
*/
class EXP TSound : public TMusicData {

    public:
        enum { yes=-2, undefined=-1 };
        EXP friend SMARTP<TSound> newSound();
        
        virtual void accept(TScoreVisitor& visitor);

        //! Tempo is expressed in quarter notes per minute.
        void setTempo(long tempo);
        //! Dynamics (or MIDI velocity) are expressed as a percentage of the default forte value (90 for MIDI).
        void setDynamics(long dyn);
       //! Dacapo indicates to go back to the beginning of the movement.
        void setDacapo(YesNo::type yn);
        //! a segno mark. The value can be used to distinguish multiple jumps.
        void setSegno(long segno); 
        //! dalsegno is used for backward jump to a segno sign. The value can be used to distinguish multiple jumps.
        void setDalsegno(long ds);
        //! a coda mark. The value can be used to distinguish multiple jumps.
        void setCoda(long coda);
        //! tocoda is used for forward jump to a coda sign. The value can be used to distinguish multiple jumps.
        void setTocoda(long tc);
        //! to be used with segno or coda attributes to avoid computation of divisions.
        void setDivisions(long div);
        //! Forward-repeat is used when a forward repeat sign is implied, and usually follows a bar line.
        void setForwardRepeat(YesNo::type yn);
        //! The fine attribute follows the final note or rest in a movement with a da capo direction. It may take the special \e yes value
        void setFine(long fine);
		//! If the sound element applies only one time through a repeat, the time-only attribute indicates which time to apply the sound element.
		void setTimeOnly(long time);
        //! Pizzicato in a sound element effects all following notes. \e true indicates pizzicato, \e false indicates arco.
        void setPizzicato(YesNo::type pizz);
/* seems to be not any more mentionned in the dtd
        //! MIDI channel indication allow for detailed control of MIDI assignment.
        void setMidiChannel(long chan);
        //! MIDI instrument indication allow for detailed control of MIDI assignment.
        void setMidiIntrument(long instr);
*/
		/*!
			Pan and elevation allow placing of sound in a 3-D space relative to the listener. 
			Both are expressed in degrees ranging from -180 to 180. For pan, 0 is straight ahead,
			-90 is hard left, 90 is hard right, and -180 and 180 are directly behind the listener. 
			For elevation, 0 is level with the listener, 90 is directly above, and -90
			is directly below.
		*/
		void setPan(long pan);
        //! see \p setPan.
		void setElevation(long el);
		/*!
			The damper-pedal, soft-pedal, and sostenuto-pedal attributes effect playback 
			of the three common piano pedals and their MIDI controller equivalents. 
			Yes indicates the pedal is depressed; no indicates the pedal is released.
		*/
		void setDamperPedal(YesNo::type yn);
        //! see \p setDamperPedal.
		void setSoftPedal(YesNo::type yn);
        //! see \p setDamperPedal.
		void setSostenutoPedal(YesNo::type yn);

        long add(const SMidiInstrument& instr);

        long getTempo() const 			{ return fTempo; }
        long getDynamics() const  		{ return fDynamics; }
        YesNo::type getDacapo() const 	{ return fDacapo; }
        long getSegno() const 			{ return fSegno; }
        long getDalsegno() const		{ return fDalsegno; }
        long getCoda() const 			{ return fCoda; }
        long getTocoda() const 			{ return fTocoda; }
        long getDivisions() const		{ return fDivisions; }
        YesNo::type getForwardRepeat() const { return fForwardRepeat; }
        long getFine() const 			{ return fFine; }
        long getTimeOnly() const		{ return fTimeOnly; }
        YesNo::type getPizzicato() const	{ return fPizzicato; }
// seems to be not any more mentionned in the dtd
//        long getMidiChannel() const 	{ return fMidiChannel; }
//        long getMidiIntrument() const	{ return fMidiIntrument; }
		long getPan() const				{ return fPan; }
		long getElevation() const		{ return fElevation; }
        YesNo::type getDamperPedal() const		{ return fDamperPedal; }
        YesNo::type getSoftPedal() const		{ return fSoftPedal; }
        YesNo::type getSostenutoPedal() const 	{ return fSostenutoPedal; }

        vvector<SMidiInstrument>& instrumentList() { return fInstrumentList; }
		        
    protected:
        TSound() { init(); }
        virtual ~TSound() {}
        void 	init();

    private:
        long fTempo;
        long fDynamics;
		YesNo::type fDacapo;
        long fSegno; 
        long fDalsegno;
        long fCoda;
        long fTocoda;
        long fDivisions;
        YesNo::type fForwardRepeat;
        long fFine;
        long fTimeOnly;
        YesNo::type fPizzicato;
// seems to be not any more mentionned in the dtd
//        long fMidiChannel;
//        long fMidiIntrument;
		long fPan;
		long fElevation;
		YesNo::type fDamperPedal;
		YesNo::type fSoftPedal;
		YesNo::type fSostenutoPedal;
        vvector<SMidiInstrument>	fInstrumentList;
};
typedef SMARTP<TSound> SSound;

} // namespace MusicXML


#endif
