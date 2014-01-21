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

#ifndef __TNote__
#define __TNote__

#include "exports.h"
#include "common.h"
#include "TPitch.h"
#include "TNotation.h"
#include "TScoreInstrument.h"
#include "TTimeModification.h"
#include "smartpointer.h"
#include "TScoreVisitor.h"

namespace MusicXML 
{

class TMultipleLyricPart;
typedef SMARTP<TMultipleLyricPart> SMultipleLyricPart;

/*!
\brief Representation of lyrics.

	Text underlays for lyrics, based on Humdrum with support
	for other formats. The lyric number indicates multiple
	lines, though a name can be used as well (as in Finale's
	verse/chorus/section specification). Word extensions are
	represented using the extend element. Hyphenation is 
	indicated by the syllabic element, which can be single, 
	begin, end, or middle. These represent single-syllable
	words, word-beginning syllables, word-ending syllables,
	and mid-word syllables. Multiple syllables on a single
	note are separated by elision elements. A hyphen in the
	text element should only be used for an actual hyphenated
	word. Humming and laughing representations are taken from
	Humdrum. The end-line and end-paragraph elements come
	from RP-017 for Standard MIDI File Lyric meta-events;
	they help facilitate lyric display for Karaoke and
	similar applications. Language names for text elements
	come from ISO 639, with optional country subcodes from
	ISO 3166.
\n
	The MusicXML \e lyric element is defined in note.dtd.

*/
class EXP TLyric : public Fontable, public Editorialable, public visitable, public smartable {

    public:
		enum syllabic { undefined = -1, single=1, begin, end, middle, last=middle };

		EXP friend SMARTP<TLyric> newLyric();

        virtual void accept(TScoreVisitor& visitor);

 		string 		getName() const		{ return fName; }
 		string 		getText() const		{ return fText; }
        int			getNumber() const	{ return fNumber; }
        syllabic	getSyllabic() const	{ return fSyllabic; }
        bool		getExtend() const	{ return fExtend; }
        bool		getLaughing() const	{ return fLaughing; }
        bool		getHumming() const	{ return fHumming; }
        bool		getEndLine() const	{ return fEndLine; }
        bool		getEndParagraph() const	{ return fEndParagraph; }
        vvector<SMultipleLyricPart>& getMultipleParts() 	{ return fMultiple; }
        
 		void 		setName(const string name);
 		void 		setText(const string text);
        void		setNumber(int number);
        void		setSyllabic(syllabic syll);
        void		setExtend(bool extend);
        void		setLaughing(bool laugh);
        void		setHumming(bool hum);
        void		setEndLine(bool el);
        void		setEndParagraph(bool ep);
        void		add (SMultipleLyricPart part);
 
	//! convert a numeric syllabic type to a MusicXML string
	static const string	xmlsyllabic (syllabic d);
	//! convert an MusicXML string to a numeric syllabic type
	static syllabic		xmlsyllabic (const string str);

	protected:
        TLyric() : fNumber(undefined), fSyllabic(undefined), 
				fExtend(false), fLaughing(false), fHumming(false), fEndLine(false), fEndParagraph(false) {}
        virtual ~TLyric() {}
	private:
        int 		fNumber;
        string		fName;
		string  	fText;
        syllabic	fSyllabic;
        vvector<SMultipleLyricPart> fMultiple;
        bool fExtend, fLaughing, fHumming, fEndLine, fEndParagraph;

	static bimap<string, syllabic> fSyllabic2String;
	static syllabic fSyllabicTbl[];
	static string 	fSyllabicStrings[];
};
typedef SMARTP<TLyric> SLyric;

/*!
\brief Representation of multiple lyrics on a single note.

    Multiple syllables on a single note are separated by elision elements.
    Elision are implicit in TMultipleLyricPart.
*/
class EXP TMultipleLyricPart : public Fontable, public visitable, public smartable {
    public:
		EXP friend SMARTP<TMultipleLyricPart> newMultipleLyricPart();
        
        virtual void accept(TScoreVisitor& visitor);
        string				getText() const		{ return fText; }
        TLyric::syllabic	getSyllabic() const	{ return fSyllabic; }
        void		setText (const string text);
        void		setSyllabic (TLyric::syllabic syll);
        
	protected:
        TMultipleLyricPart() : fSyllabic(TLyric::undefined) {}
        virtual ~TMultipleLyricPart() {}
    private:
		string				fText;
        TLyric::syllabic	fSyllabic;
};
typedef SMARTP<TMultipleLyricPart> SMultipleLyricPart;

/*!
\brief Represents the expression context of a note.

	TExpression corresponds to a subset of the \e note attributes as defined
	in notes.dtd
*/
class EXP TExpression : public visitable, public smartable {

    public:
		enum { undefined = -1 };
		EXP friend SMARTP<TExpression> newExpression();

        virtual void accept(TScoreVisitor& visitor);

        //! reflects the MIDI key on velocity (expressed in terms of percentages of a standard MIDI forte volume of 90)
		void 	setDynamic(long dyn);
        //! reflects the MIDI key off velocity (expressed in terms of percentages of a standard MIDI forte volume of 90)
		void 	setEndDynamic(long dyn);
        //! used to alter the starting time of the note (expressed in term of divisions)
		void 	setAttack(long attack);
        //! used to alter the stopping time of the note (expressed in term of divisions)
		void 	setRelease(long rel);
		//! used when just this note is sounded pizzicato
		void	setPizzicato(YesNo::type pizz);

		long 	getDynamic() const		{ return fDynamic; }
		long 	getEndDynamic() const	{ return fEndDynamic; }
		long 	getAttack() const		{ return fAttack; }
		long 	getRelease() const		{ return fRelease; }
		YesNo::type	getPizzicato() const { return fPizzicato; }

	protected:
        TExpression() 
			: 	fDynamic(undefined), fEndDynamic(undefined), fAttack(undefined), 
				fRelease(undefined), fPizzicato(YesNo::undefined) {}
        virtual ~TExpression() {}
	private:
		long 	fDynamic;
		long 	fEndDynamic;
		long 	fAttack;
		long 	fRelease;
		YesNo::type	fPizzicato;
};
typedef SMARTP<TExpression> SExpression;

/*!
\brief Representation of an accidental.

	Actual notated accidentals. Valid values include: sharp,
	natural, flat, double-sharp, sharp-sharp, flat-flat,
	natural-sharp, natural-flat, quarter-flat, quarter-sharp,
	three-quarters-flat, and three-quarters-sharp. Editorial
	and cautionary indications, are indicated by attributes.
	Values for these attributes are "no" if not present.
\n
	The MusicXML \e accidental element is defined in note.dtd.
*/
class EXP TAccidental : public visitable, public smartable {

    public:
		enum accident { undefined = -1, 
				sharp=1, natural, flat, double_sharp, sharp_sharp, flat_flat,
				natural_sharp, natural_flat, quarter_flat, quarter_sharp,
				three_quarters_flat, three_quarters_sharp, last=three_quarters_sharp };

		EXP friend SMARTP<TAccidental> newAccidental();

        virtual void accept(TScoreVisitor& visitor);

 		accident 	getAccident() const		{ return fAccident; }
		YesNo::type getCautionary() const	{ return fCautionary; }
		YesNo::type getEditorial() const	{ return fEditorial; }
		FullCue::type getSize() const		{ return fSize; }

		void setAccident(accident acc);
		void setCautionary(YesNo::type yn);
		void setEditorial(YesNo::type yn);
		void setSize(FullCue::type size);
 
	//! convert a numeric accidental type to a MusicXML string
	static const string	xmlaccidental (accident d);
	//! convert an MusicXML string to a numeric accidental type
	static accident		xmlaccidental (const string str);

	protected:
        TAccidental() :
			 	fAccident(undefined), fCautionary(YesNo::undefined), 
				fEditorial(YesNo::undefined), fSize(FullCue::undefined) {}
        virtual ~TAccidental() {}
	private:
		accident 	fAccident;
		YesNo::type fCautionary;
		YesNo::type fEditorial;
		FullCue::type fSize;

	static bimap<string, accident> fAccident2String;
	static accident fAccidentTbl[];
	static string 	fAccidentStrings[];
};
typedef SMARTP<TAccidental> SAccidental;

/*!
\brief the shape of the note head

	The notehead element indicates shapes other than the
	open and closed ovals associated with note durations.
	The element value can be slash, triangle, diamond,
	square, cross, x, circle-x, normal, or none.
	For the enclosed shapes, the default is to be hollow for
	half notes and longer, and filled otherwise. The filled
	attribute can be set to change this if needed.
	If the parentheses attribute is set to yes, the notehead
	is parenthesized. It is no by default.
\n
	The MusicXML \e notehead element as defined in note.dtd.
*/
class EXP TNoteHead : public visitable, public smartable {

    public:
		enum notehead { undefined = -1, 
				slash=1, triangle, diamond, square, cross, x, circle_x, normal, none, last=none };
		EXP friend SMARTP<TNoteHead> newNoteHead();

        virtual void accept(TScoreVisitor& visitor);

		void 	setShape(notehead shape);
		void 	setFilled(YesNo::type filled);
		void 	setParentheses(YesNo::type par);

		notehead 	getShape() const		{ return fShape; }
		YesNo::type getFilled() const		{ return fFilled; }
		YesNo::type getParentheses() const	{ return fParentheses; }

	//! convert a numeric beam type to a MusicXML string
	static const string	xmlnotehead (notehead d);
	//! convert an MusicXML string to a numeric beam type
	static notehead		xmlnotehead (const string str);

	protected:
        TNoteHead() : fShape(undefined), fFilled(YesNo::undefined), fParentheses(YesNo::undefined) {}
        virtual ~TNoteHead() {}
	private:
		notehead 	fShape;
		YesNo::type fFilled;
		YesNo::type fParentheses;

	static bimap<string, notehead> fNoteHead2String;
	static notehead 	fNoteHeadTbl[];
	static string 		fNoteHeadStrings[];
};
typedef SMARTP<TNoteHead> SNoteHead;

/*!
\brief beaming representation.

	Beam types include begin, continue, end, forward hook,
	and backward hook. In MuseData, up to six concurrent
	beams are available to cover up to 256th notes. This
	seems sufficient so we use an enumerated type defined
	in common.dtd. The repeater attribute needs to be 
	specified with a "yes" value for each beam using it.
\n
	The MusicXML \e beam element as defined in note.dtd.
*/
class EXP TBeam : public visitable, public smartable {

    public:
		enum beamtype { undefined = -1, 
				beambegin=1, beamcontinue, beamend, beamforward, beambackward, last=beambackward };

		EXP friend SMARTP<TBeam> newBeam();

		beamtype 		getType() const			{ return fType; }
		int 			getNumber() const		{ return fNumber; }
		YesNo::type 	getRepeater() const		{ return fRepeater; }

		void 		setType(beamtype type);
		void 		setNumber(int num);
		void 		setRepeater(YesNo::type r);

        virtual void accept(TScoreVisitor& visitor);

	//! convert a numeric beam type to a MusicXML string
	static const string	xmlbeamtype (beamtype d);
	//! convert an MusicXML string to a numeric beam type
	static beamtype		xmlbeamtype (const string str);

	protected:
        TBeam() : fType(undefined), fNumber(undefined), fRepeater(YesNo::undefined) {}
        virtual ~TBeam() {}
	private:
		beamtype 		fType;
		int 			fNumber;
		YesNo::type 	fRepeater;

	static bimap<string, beamtype> fBeam2String;
	static beamtype 	fBeamTbl[];
	static string 		fBeamStrings[];
};
typedef SMARTP<TBeam> SBeam;

/*!
\brief Represents the graphic elements of a note.

	Graphic elements of a note represents the note type
	(whole, quarter, etc...), possible dots, accidentals,
	tuplets notation, stems, notehead, staff, beams and other
	notation elements.

\todo the MusicXML stem definition includes now a position attribute and should 
be implemented as a separate element
*/
class EXP TGraphNote : public Editorialable, public visitable, public smartable {

    public:
		enum { undefined = -1, stemdown=1, stemup, stemnone, stemdouble, last=stemdouble  };
		
		EXP friend SMARTP<TGraphNote> newGraphNote();

        virtual void accept(TScoreVisitor& visitor);

		//! the corresponding note voice
		void	setVoice(long voice);
		//! the graphical note sign (whole, half, quarter, ...) corresponds to the \e type element
		void	setType(NoteType::type type);
		//! the note size (full or cue)
		void	setSize(FullCue::type size);
		//! the number of dots
		void 	setDots(unsigned long dots);
		//! stems can be down, up, none, or double. 
		void	setStem(long stem);
		//! staff assignment (only needed for music notated on the grand staff)
		void	setStaff(long staff);

		long			getVoice() const	{ return fVoice; }
		NoteType::type	getType() const		{ return fType; }
		FullCue::type	getSize() const		{ return fSize; }
		unsigned int 	getDots() const		{ return fDots; }
		int 			getStem() const		{ return fStem; }
		int				getStaff() const	{ return fStaff; }
        
        long add(const SBeam& beam);
        long add(const SNotation& notation);

		//! graphical accidentals 
		SAccidental&	accidental()			{ return fAccidental; }
		//! graphical element for tuplets 
		STimeModification& timemodification()	{ return fTimeMod; }
		//! the shape of the note head
		SNoteHead&		notehead()				{ return fNoteHead; }
		//! beaming definition
		vvector<SBeam>& 	beamList()				{ return fBeamList; }
		//! notations elements (accents, slurs, etc...)
		vvector<SNotation>& 	notationList()		{ return fNotationList; }
		//! the printout entity
		TPrintout&	printout()						{ return fPrintout; }

	//! convert an integer stem attribute to a MusicXML string
	static const string	xmlstem (int d);
	//! convert an MusicXML string to an integer stem attribute
	static int				xmlstem (const string str);

	protected:
        TGraphNote()
			: fVoice(undefined), fType(NoteType::undefined), fSize(FullCue::undefined), fDots(0),
			  fStem(undefined), fStaff(undefined) {}
        virtual ~TGraphNote() {}
	private:
		long			fVoice;
		NoteType::type	fType;
		FullCue::type	fSize;
		unsigned long 	fDots;
		SAccidental		fAccidental;
		STimeModification fTimeMod;
		long 			fStem;
		SNoteHead		fNoteHead;
		long				fStaff;
		vvector<SBeam> 		fBeamList;
		vvector<SNotation> 	fNotationList;
		TPrintout		fPrintout;

	static bimap<string, int> fStem2String;
	static int 		fStemTbl[];
	static string 	fStemStrings[];
};
typedef SMARTP<TGraphNote> SGraphNote;

/*!
\brief grace note representation.

	The slash attribute for a grace note is yes for slashed eighth notes. 
	The other grace note attributes come from MuseData sound suggestions.
	Steal-time-previous indicates the percentage of time to steal from 
	the previous note for the grace note. Steal-time-following indicates 
	the percentage of time to steal from the following note for the grace 
	note. Make-time indicates to make time, not steal time; the
	units are in real-time divisions for the grace note. 
\n
	The MusicXML \e grace element is defined in note.dtd.
*/
class EXP TGraceNote : public visitable, public smartable {

    public:
		enum { undefined = -1 };
		EXP friend SMARTP<TGraceNote>	newGraceNote();

        virtual void accept(TScoreVisitor& visitor);

		int getStealPrevious() const	{ return fStealPrevious; }
		int getStealFollowing() const	{ return fStealFollowing; }
		int getMakeTime() const			{ return fMakeTime; }
		YesNo::type getSlash() const	{ return fSlash; }

		void setStealPrevious(int v);
		void setStealFollowing(int v);
		void setMakeTime(int v);
		void setSlash(YesNo::type v);

	protected:
        TGraceNote() :
			fStealPrevious(undefined), fStealFollowing(undefined), fMakeTime(undefined),
			fSlash(YesNo::undefined) {}
        virtual ~TGraceNote() {}
	private:
		int fStealPrevious;
		int fStealFollowing;
		int fMakeTime;
		YesNo::type fSlash;
};
typedef SMARTP<TGraceNote> SGraceNote;

class TNote;
typedef SMARTP<TNote> SNote;
/*!
\brief TNote is the main common type of the music representation.

	It is derived from the MusicXML note element as described in note.dtd
	but only the sounding attributes has been retained at core level. 
	All the other elements are stored in contextual
	information which is divided in 2 parts: 
	\li the graphical notation context (like voice, notehead, accidentals etc...),  
		available using the graphics() method
	\li the expression context (such as dynamics, attack, release etc...),
		available using the expression() method
*/
class EXP TNote : public TMusicData, public Positionable {

    public:
		enum style { normal, cue, grace };
		enum type { pitched, rest, unpitched };
		EXP friend SMARTP<TNote>	newNote();

        virtual void accept(TScoreVisitor& visitor);

        //! set the note duration, specified in division units
		void			setDuration (unsigned long dur);
        //! gives the note duration
		unsigned long	getDuration () const			{ return fDuration; }
        //! gives the sounding duration ie takes account of possible ties
        unsigned long 	getFullDuration();
        //! the score-instrument (if any)
		void  			setInstrumentID(string id);
		const string  	getInstrumentID() const			{ return fInstrumentID; }

		//! to set the style of the note (normal, cue or grace - defaults to normal)
		void	 		setStyle(style t);
		style	 		getStyle() const				{ return fStyle; }

		//! to set the type of the note (regular, rest, unpitched - defaults to regular)
		void	 		setType(type t);
		type	 		getType() const					{ return fType; }

        //! the lyrics
		vvector<SLyric>		getLyrics()			{ return fLyrics; }
        //! add a lyric to the note
		void				add(SLyric lyric);

        //! the note pitch. May also be used for unpitched and rest placement.
		SPitch&				pitch()			{ return fPitch; }
        //! the expression context of the note, expressed in terms of current division
		SExpression&		expression()	{ return fExpression; }
        //! the optional previous tied note (corresponds to the \e tie element)
		SNote& 				tiePrevious()	{ return fTiePrev; }
        //! the optional next tied note (corresponds to the \e tie element)
		SNote& 				tieNext()		{ return fTieNext; }
        //! the graphical attributes of the note
		SGraphNote&			graphics()		{ return fGraphNote; }
        //! the grace note optionnal element
		SGraceNote&			graceNote()			{ return fGraceNote; }
        
        //! the note is the first of a group of tied notes
        bool isTie() 	{ return tieNext() && !tiePrevious();}
        
        //! the note is "single", not part of a tied group
        bool isSingle() { return !tieNext() && !tiePrevious();}
        
        bool isNormal() { return fStyle == normal;}
        bool isGrace() 	{ return fStyle == grace;}
        bool isCue() 	{ return fStyle == cue;}
        
        bool isPitched() { return fType == pitched;}
        bool isRest() 	{ return fType == rest;}
        bool isUnpitched() { return fType == unpitched;}

		long getVoice() const;
    
	protected:
        TNote() : fStyle(normal), fType(pitched), fDuration(1), fTiePrev(NULL), fTieNext(NULL)  {}
        virtual ~TNote() {}

	private:
		style			fStyle;
		type			fType;
		SPitch			fPitch;
		unsigned long	fDuration;
		string  		fInstrumentID;

		SExpression			fExpression;
		SNote 				fTiePrev;
		SNote 				fTieNext;
		SGraphNote			fGraphNote;
		SGraceNote			fGraceNote;
		vvector<SLyric>		fLyrics;
};

} // namespace MusicXML


#endif
