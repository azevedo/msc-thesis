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

#ifndef __TIdentification__
#define __TIdentification__

#include "exports.h"
#include "conversions.h"
#include "TScoreVisitor.h"

#include <string>

namespace MusicXML 
{

/*!
\brief fields of miscellaneous identification
*/
class EXP TMiscField : public visitable, public smartable {
    public:
        EXP friend SMARTP<TMiscField> newMiscField(std::string name);

		virtual void accept(TScoreVisitor& visitor);
		void 	setData(const std::string& type);
        void 	setName(const std::string& name);
		const std::string& 	getData() const 	{ return fData; }
        const std::string& 	getName() const 	{ return fName; }

   protected:
	   TMiscField(std::string name) : fName(name) {} // name is a required attribute
        virtual ~TMiscField() {}
   private:
        std::string 	fName;
		std::string 	fData;
};
typedef SMARTP<TMiscField> SMiscField;

/*!
\brief miscellaneous identification. 

	If a program has other metadata not yet supported
	in MusicXML, it can go in the miscellaneous area. 
*/
class EXP TMiscId : public visitable, public smartable {
    public:
        EXP friend SMARTP<TMiscId> newMiscId();

		virtual void accept(TScoreVisitor& visitor);
        void 	add(SMiscField field);
		vvector<SMiscField>& 	getFields() { return fFields; }

   protected:
	   TMiscId() {}
        virtual ~TMiscId() {}
   private:
		vvector<SMiscField>  fFields;
};
typedef SMARTP<TMiscId> SMiscId;

/*!
\brief Represents the author(s) of the score . 

	The type attribute can be used to distinguish words and music so
	as to represent lyricists and librettists as well as
	composers. Thus, there can be multiple creators in an
	identification. It corresponds to the MusicXML 
	\e creator element as defined in identity.dtd.
*/
class EXP TCreator : public visitable, public smartable {
    public:
        EXP friend SMARTP<TCreator> newCreator();

		virtual void accept(TScoreVisitor& visitor);

		void 	setType(const std::string& type);
        void 	setName(const std::string& name);

		const std::string& 	getType() const 	{ return fType; }
        const std::string& 	getName() const 	{ return fName; }

   protected:
	   TCreator() {}
        virtual ~TCreator() {}
   private:
        //! an impled attribute to differentiate creators
		std::string 	fType;
        //! the creator name
        std::string 	fName;
};
typedef SMARTP<TCreator> SCreator;

/*!
\brief Contains copyright and other intellectual property notices.

	Words, music, and derivatives can have different types,
	so multiple rights tags with different type attributes
	are supported. It corresponds to the MusicXML 
	\e right element as defined in identity.dtd.
*/
class EXP TRights : public visitable, public smartable {
    
    public:
        EXP friend SMARTP<TRights> newRights();
        
		virtual void accept(TScoreVisitor& visitor);
		const std::string& 	getType() const		{ return fType; }
        const std::string& 	getRight() const	{ return fRights; }
        
		void 	setType(const std::string& type);
        void 	setRights(const std::string& rights);

    protected:
        TRights() {}
        virtual ~TRights() {}
    private:
        //! an impled attribute to differentiate multiple rights
		std::string 	fType;
        std::string 	fRights;
};
typedef SMARTP<TRights> SRights;

/*!
\brief the MusciXML \e support element.

	The supports element indicates if the encoding supports
	a particular MusicXML element. This is recommended for
	elements like beam, stem, and accidental, where the
	absence of an element is ambiguous if you do not know
	if the encoding supports that element.
*/
class EXP TSupport : public visitable, public smartable {
    
    public:
        EXP friend SMARTP<TSupport> newSupport(YesNo::type type, const string& element);
        
		virtual void accept(TScoreVisitor& visitor);
		YesNo::type			getType() const		{ return fType; }
        const std::string& 	getElement() const	{ return fElement; }
        
		void 	setType(YesNo::type type);
        void 	setElement(const std::string& element);

    protected:
        TSupport(YesNo::type type, const string& element) : fType(type), fElement(element) {}
        virtual ~TSupport() {}
    private:
		YesNo::type 	fType;
        std::string 	fElement;
};
typedef SMARTP<TSupport> SSupport;

/*!
\brief Contains information about encoding.

	Encoding contains information about who did the digital
	encoding, when, with what software, and in what aspects.
	The supports element indicates if the encoding supports
	a particular MusicXML element. This is recommended for
	elements like beam, stem, and accidental, where the
	absence of an element is ambiguous if you do not know
	if the encoding supports that element.

	It corresponds to the MusicXML \e encoding element as defined 
	in identity.dtd.
*/
class EXP TEncoding : public visitable, public smartable {
    
    public:		
        EXP friend SMARTP<TEncoding> newEncoding();

		virtual void accept(TScoreVisitor& visitor);
		
		void setDate (const std::string& date);
		void setEncoder (const std::string& encoder);
		void setEncoderType (const std::string& etype);
		void setSoftware (const std::string& soft);
		void setDescription (const std::string& desc);

		const std::string& getDate () const			{ return fDate; }
		const std::string& getEncoder () const		{ return fEncoder; }
		const std::string& getEncoderType () const	{ return fEncoderType; }
		const std::string& getSoftware () const		{ return fSoftware; }
		const std::string& getDescription () const	{ return fDescription; }

		void				add (SSupport support);
		vvector<SSupport>&  support()				{ return fSupport; }

   protected:
        TEncoding() {}
        virtual ~TEncoding() {}
   private:
        //! an optionnal encoding date
        std::string 	fDate;
        //! the encoder name
        std::string	fEncoder;
        //! an impled attribute to differentiate encoder types
        std::string 	fEncoderType;
        //! the software used for encoding
        std::string 	fSoftware;
        //! encoding description
        std::string 	fDescription;
        //! supported elements
		vvector<SSupport> fSupport;
};
typedef SMARTP<TEncoding> SEncoding;

/*!
\brief Contains metadata about the score. 

	It includes information from the initial 13+ records in a
	MuseData file that may apply at a score-wide, movement-
	wide, or part-wide level. It corresponds to the MusicXML 
	\e identification element as defined in identity.dtd.
*/
class EXP TIdentification : public visitable, public smartable {
    
    public:
        EXP friend SMARTP<TIdentification> newIdentification();

		virtual void accept(TScoreVisitor& visitor);

		long add (const SCreator& creator);
		long add (const SRights& rights);
		void setSource (const std::string& src);
		
        vvector<SCreator>& 	creators()		{ return fCreatorList; }
        vvector<SRights>& 	rights()		{ return fRightsList; }
        const std::string& 	getSource()		{ return fSource; }

        SEncoding&			encoding()		{ return fEncoding; }
        SMiscId&			misc()			{ return fMiscellaneaous; }

    protected:
        TIdentification() {}
        virtual ~TIdentification() {}
    private:
        vvector<SCreator>	fCreatorList;
        vvector<SRights>	fRightsList;
        SEncoding 			fEncoding;
        std::string 		fSource;
		SMiscId				fMiscellaneaous;
};
typedef SMARTP<TIdentification> SIdentification;

} // namespace MusicXML


#endif
