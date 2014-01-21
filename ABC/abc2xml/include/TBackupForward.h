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

#ifndef __TBackupForward__
#define __TBackupForward__

#include "exports.h"
#include "common.h"

namespace MusicXML 
{

/*!
\brief Represents the MusicXML backup element.

	Backup and forward are required to coordinate multiple
	voices in one part, including music on a grand staff.
	Backup is generally used to move between voices and staves. 
	Thus the backup element does not include voice or staff
	elements. Duration values should always be positive,
	and should not cross measure boundaries.
\n	The MusicXML \e backup element is defined in note.dtd.
*/
class EXP TBackup : public TMusicData, public Editorialable {
    public:
        EXP friend SMARTP<TBackup> newBackup();
       
        virtual void accept(TScoreVisitor& visitor);        
        //! Duration is a required element and is set to zero by the constructor
		void	setDuration (long dur);
		long	getDuration ()	const		{ return fDur; }

    protected:
		TBackup() : fDur(0) {}    		
        virtual ~TBackup() {}

	private:
		//! required duration element
		long fDur;
};
typedef SMARTP<TBackup> SBackup;

/*!
\brief Represents the MusicXML forward element.

	Backup and forward are required to coordinate multiple
	voices in one part, including music on a grand staff.
	Forward is generally used within voices and staves. 
	Duration values should always be positive,
	and should not cross measure boundaries.
\n	The MusicXML \e forward element is defined in note.dtd.
*/
class EXP TForward : public TMusicData, public Editorialable {
    public:
        enum { undefined=-1 };
    
        EXP friend SMARTP<TForward> newForward();
       
        virtual void accept(TScoreVisitor& visitor);

        //! Duration is a required element and is set to zero by the constructor
        TForward&	setDuration (long dur);
        TForward&	setVoice 	(long voice);
        TForward&	setStaff 	(long staff);
        long		getDuration ()	const		{ return fDur; }
        long		getVoice 	()	const		{ return fVoice; }
        long		getStaff 	()	const		{ return fStaff; }

    protected:
        TForward() : fDur(0), fVoice(undefined), fStaff(undefined) {}
        virtual ~TForward() {}
    
    private:
		//! required duration element
        long fDur;
		//! optionnal editorial voice element
        long fVoice;
		//! optionnal editorial staff element
        long fStaff;
};
typedef SMARTP<TForward> SForward;

} // namespace MusicXML

#endif
