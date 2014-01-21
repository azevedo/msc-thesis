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

#ifndef __common__
#define __common__

#include "exports.h"
#include "TScoreVisitor.h"
#include "bimap.h"
#include "conversions.h"
#include "smartpointer.h"
#include <string>

using namespace std;

namespace MusicXML 
{

/*!
\brief base class for the elements of a \e part-list.

	TPartListElement is an abstract class which purpose is to define
	a common type for all the elements of a \e part-list element.
*/
class TPartListElement : public virtual visitable, public virtual smartable {
     protected:
        TPartListElement () {}
        virtual ~TPartListElement() {}
};
typedef SMARTP<TPartListElement> 	SPartListElement;

/*!
\brief base class for all the elements of the \e notation element.

	TNotationElement is an abstract class which purpose is to define
	a common type for all the elements of a \e notation element.
*/
class TNotationElement : public virtual visitable, public virtual smartable {
     protected:
        TNotationElement () {}
        virtual ~TNotationElement() {}
};
typedef SMARTP<TNotationElement> 	SNotationElement;

/*!
\brief base class for all the elements of the \e direction-type element.

	TDirectionTypeElement is an abstract class which purpose is to define
	a common type for all the elements of a \e direction-type element.
*/
class TDirectionTypeElement : public virtual visitable, public virtual smartable {
     protected:
        TDirectionTypeElement () {}
        virtual ~TDirectionTypeElement() {}
};
typedef SMARTP<TDirectionTypeElement> 	SDirectionTypeElement;

/*!
\brief base class for all the elements of the \e music-data entity.

	TMusicData is an abstract class which purpose is to define
	a common type for all the elements covered by the \e music-data entity.
*/
class TMusicData : public virtual visitable, public virtual smartable {
     protected:
        TMusicData () {}
        virtual ~TMusicData() {}
};
typedef SMARTP<TMusicData> 	SMusicData;

typedef long	position[4];

/*!
\brief 	Represents a MusicXML \e position entity

    The position attributes are based on MuseData print
	suggestions. For most elements, any program will compute
	a default x and y position. The position attribute lets
	this be changed two ways. The default-x and default-y
	attributes change the computation of the default
	position. The origin becomes the left-hand side of
	the note or the musical position within the bar (x)
	and the top line of the staff (y). The relative-x and
	relative-y attributes change the position relative to
	the default position, either as computed by the
	individual program, or as overridden by the default-x
	and default-y attributes.
\n	
	Positive x is right, negative x is left; positive y
	is up, negative y is down. All units are in tenths of
	interline space. Positions can be applied to notes,
	notations, directions, and stems. For stems, positive y
	lengthens a stem while negative y shortens it. Negative
	values for default-y are not allowed.
\n	
	\e position is defined in common.dtd
*/
class EXP TPosition {
    public:
        enum { undefined=0, defaultx=1, defaulty, relativex, relativey, last=relativey };
        
        TPosition() { fPosition[0]=fPosition[1]=fPosition[2]=fPosition[3]= undefined; }
        virtual ~TPosition() {}
                
        virtual void accept(TScoreVisitor& visitor);
        void set (int pos, long val)	{ if ((pos>0) && (pos<=last)) fPosition[pos-1] = val; }
        long get (int pos) const	{ return ((pos>0) && (pos<=last)) ? fPosition[pos-1] : undefined; }

	//! convert an integer position to a MusicXML string
	static const string	xmlpos (int d);
	//! convert an MusicXML string to an integer position
	static int			xmlpos (const string str);

    private:
        //! all the attributes are implied
        position fPosition;

	static bimap<string, int> fPos2String;
	static int 		fPosTbl[];
	static string 	fPosStrings[];
};

/*!
\brief base class for all elements that have a position.
*/

class EXP Positionable {
    public:
        Positionable() {}
        virtual ~Positionable() {}

        TPosition& position() 					{ return fPosition; }
    
    private:
        TPosition fPosition;
};

/*!
\brief Represents the MusicXml \e placement entity

    It is defined in common.dtd.
*/
class EXP TPlacement {
    public:
        enum type { undefined, above, below, last=below };

        TPlacement(int place=undefined) : fPlacement(place) {}
        virtual ~TPlacement() {}

        void accept(TScoreVisitor& visitor);
        void set (int place) 		{ fPlacement = place; }
        int  get () const			{ return fPlacement; }

	//! convert an integer placement to a MusicXML string
	static const string	xmlplace (int d);
	//! convert an MusicXML string to an integer placement
	static int			xmlplace (const string str);

    private:
        int fPlacement;

	static bimap<string, int> fPlace2String;
	static int 		fPlaceTbl[];
	static string 	fPlaceStrings[];
};

/*!
\brief base class for all elements that have a placement.
*/

class EXP Placementable {
    public:
        Placementable() {}
        virtual ~Placementable() {}
        TPlacement& placement() 				{ return fPlacement; }
    
    private:
        TPlacement fPlacement;
};

/*!
\brief Represents the MusicXml \e orientation entity

	The orientation attribute indicates whether slurs and
	ties are overhand (tips down) or underhand (tips up).
    It is defined in common.dtd.
*/
class EXP TOrientation {
    public:
        enum type { undefined, over, under, last = under };

        TOrientation(int orient=undefined) : fOrientation(orient) {}
        virtual ~TOrientation() {}

        virtual void accept(TScoreVisitor& visitor);
        void set (int orient)	{ fOrientation = orient; }
        int  get () const		{ return fOrientation; }

	//! convert an integer orientation to a MusicXML string
	static const string	xmlorientation (int d);
	//! convert an MusicXML string to an integer orientation
	static int			xmlorientation (const string str);

    private:
        int fOrientation;

	static bimap<string, int> fOr2String;
	static int 		fOrTbl[];
	static string 	fOrStrings[];        
};

/*!
\brief base class for all elements that have an orientation.
*/
class EXP Orientable {
    public:
        Orientable() {}
        virtual ~Orientable() {}
        TOrientation& orientation()  { return fOrientation; }
    private:
        TOrientation fOrientation;
};

/*!
\brief base class for all the PCDATA simple elements.
*/
class TPCData : public virtual visitable, public virtual smartable {
	public:
		EXP friend SMARTP<TPCData> newPCData(const std::string eltname);
        virtual void accept(TScoreVisitor& visitor);

		void setData(const std::string& s) 		{ fData = s; }
		const std::string&  getData() const		{ return fData; }
		const std::string&  getName() const		{ return fName; }

	protected:
        TPCData (const std::string eltname) : fName(eltname) {}
        virtual ~TPCData() {}
    private:
		std::string	fName;
		std::string	fData;
};
typedef SMARTP<TPCData> 	SPCData;

/*!
\brief the footnote elements.
*/
class TFootnote : public TPCData {
	public:
		EXP friend SMARTP<TFootnote> newFootnote();
        virtual void accept(TScoreVisitor& visitor);
	protected:
        TFootnote () : TPCData("footnote") {}
        virtual ~TFootnote() {}
};
typedef SMARTP<TFootnote> 	SFootnote;

/*!
\brief the voice elements.
*/
class TVoice : public TPCData {
	public:
		EXP friend SMARTP<TVoice> newVoice();
        virtual void accept(TScoreVisitor& visitor);
	protected:
        TVoice () : TPCData("voice") {}
        virtual ~TVoice() {}
};
typedef SMARTP<TVoice> 	SVoice;

/*!
\brief the level elements.
*/
class TLevel : public TPCData {
	public:
		EXP friend SMARTP<TLevel> newLevel();
        virtual void accept(TScoreVisitor& visitor);

		void setParentheses(YesNo::type t)			{ fParentheses = t; }
		void setBrackets(YesNo::type t)				{ fBrackets = t; }
		void setSize(FullCue::type t)				{ fSize = t; }

		YesNo::type			getParentheses() const  { return fParentheses; }
		YesNo::type			getBrackets() const		{ return fBrackets; }
		FullCue::type		getSize() const			{ return fSize; }

	protected:
        TLevel () : TPCData("level") {}
        virtual ~TLevel() {}
    private:
		YesNo::type fParentheses;
		YesNo::type fBrackets;
		FullCue::type fSize;
};
typedef SMARTP<TLevel> 	SLevel;


/*!
\brief Represents the MusicXml \e editorial and editorial-voice entities

	Footnote and level are used to specify editorial
	information. These elements are used
	throughout the component MusicXML DTDs. The attributes
	allow specification of three common ways to indicate
	the editorial difference: putting parentheses or
	square brackets around a symbol, or making the symbol
	cue-size vs. full-size. If not specified, they are 
	left to application defaults.
    It is defined in common.dtd.
*/
class EXP TEditorial {
    public:
        TEditorial() {}
        virtual ~TEditorial() {}
        virtual void accept(TScoreVisitor& visitor);

		SFootnote&	footnote()	{ return fFootnote; }
		SLevel&		level()		{ return fLevel; }
		SVoice&		voice()		{ return fVoice; }

    private:
		SFootnote	fFootnote;
		SLevel		fLevel;
		SVoice		fVoice;
};

/*!
\brief base class for all elements that have editorial information.
*/
class EXP Editorialable {
    public:
        Editorialable() {}
        virtual ~Editorialable() {}
        TEditorial& editorial()  { return fEditorial; }
		TEditorial* editorialPtr()  { return &fEditorial; }
    private:
        TEditorial fEditorial;
};

/*!
\brief Represents the MusicXml \e foFontnt entity

    It is defined in common.dtd.
*/
class EXP TFont {
    public:
				 TFont() {}
        virtual ~TFont() {}

        void accept(TScoreVisitor& visitor);
        void setFamily (const std::string& family)  { fFamily = family; }
        void setStyle (const std::string& style)	{ fStyle = style; }
        void setSize (const std::string& size)		{ fSize = size; }
        void setWeight (const std::string& weight)  { fWeight = weight; }

        const std::string& getFamily () const		{ return fFamily; }
        const std::string& getStyle () const			{ return fStyle; }
        const std::string& getSize () const			{ return fSize; }
        const std::string& getWeight () const		{ return fWeight; }

    private:
		std::string 	fFamily;
		std::string 	fStyle;
		std::string 	fSize;
		std::string 	fWeight;
};

/*!
\brief base class for all elements that have a font.
*/
class EXP Fontable {
    public:
        Fontable() {}
        virtual ~Fontable() {}
        TFont& font() 				{ return fFont; }
    private:
        TFont fFont;
};

/*!
\brief Represents the trill-sound entity

	The MusicXML \e trill-sound entity is defined in common.dtd
*/
class EXP TTrillSound {
    public:
        enum { undefined, upright, inverted, last=inverted };

        TTrillSound() : fStartNote(TrillStart::undefined), fTrillStep(TrillStep::undefined), 
				fTwoNoteTurn(TrillStep::undefined), fAccelerate(YesNo::undefined),
				fBeats(0), fSecondBeat(0), fLastBeat(0) {}
        virtual ~TTrillSound() {}

        virtual void accept(TScoreVisitor& visitor);

        void setStartNote (TrillStart::type start);
        void setTrillStep (TrillStep::type step);
        void setTwoNoteTurn (TrillStep::type tnt);
        void setAccelerate (YesNo::type yn);
        void setBeat (int beat);
        void setSecondBeat (int beat);
        void setLastBeat (int beat);

        TrillStart::type getStartNote () const		{ return fStartNote; }
        TrillStep::type getTrillStep () const		{ return fTrillStep; }
        TrillStep::type getTwoNoteTurn () const		{ return fTwoNoteTurn; }
        YesNo::type getAccelerate () const			{ return fAccelerate; }
        int		getBeat () const				{ return fBeats; }
        int		getSecondBeat () const			{ return fSecondBeat; }
        int		getLastBeat () const			{ return fLastBeat; }

    private:
		TrillStart::type fStartNote;
		TrillStep::type  fTrillStep;
		TrillStep::type  fTwoNoteTurn;
		YesNo::type		 fAccelerate;
		int		fBeats, fSecondBeat, fLastBeat;
};

/*!
\brief base class for all elements that carry the trill-sound entity.
*/
class EXP Trillable {
    public:
        Trillable() {}
        virtual ~Trillable() {}
        TTrillSound& trill()  { return fTrill; }
    private:
        TTrillSound fTrill;
};

/*!
\brief Represents the bend-sound entity

	The MusicXML \e bend-sound entity is defined in common.dtd
*/
class EXP TBendSound {
    public:
        TBendSound() :fAccelerate(YesNo::undefined),
				fBeats(0), fFirstBeat(0), fLastBeat(0) {}
        virtual ~TBendSound() {}

        virtual void accept(TScoreVisitor& visitor);

        void setAccelerate (YesNo::type yn);
        void setBeat (int beat);
        void setFirstBeat (int beat);
        void setLastBeat (int beat);

        YesNo::type getAccelerate () const		{ return fAccelerate; }
        int		getBeat () const				{ return fBeats; }
        int		getFirstBeat () const			{ return fFirstBeat; }
        int		getLastBeat () const			{ return fLastBeat; }

    private:
		YesNo::type		 fAccelerate;
		int		fBeats, fFirstBeat, fLastBeat;
};

/*!
\brief base class for all elements that carry the trill-sound entity.
*/
class EXP Bendable {
    public:
        Bendable() {}
        virtual ~Bendable() {}
        TBendSound& bendSound()  { return fBend; }
    private:
        TBendSound fBend;
};

/*!
\brief Represents the fermata sign

	Fermata elements can be applied both to
	notes and to measures. Fermata type is upright if not specified.
\n 
	The MusicXML \e fermata element is defined in common.dtd
*/
class EXP TFermata : public TNotationElement, public Positionable {

    public:
        enum { undefined, upright, inverted, last=inverted };

        EXP friend SMARTP<TFermata> newFermata();

        virtual void accept(TScoreVisitor& visitor);
        void setType (int type)		{ fType = type; }
        int getType () const		{ return fType; }

	//! convert an integer type to a MusicXML string
	static const string	xmltype (int d);
	//! convert an MusicXML string to an integer type
	static int			xmltype (const string str);

    protected:
        TFermata() : fType(undefined) {}
        virtual ~TFermata() {}

    private:
        int 		fType;

	static bimap<string, int> fType2String;
	static int 		fTypeTbl[];
	static string 	fTypeStrings[];        
};
typedef SMARTP<TFermata> SFermata;

/*!
\brief Represents the MusicXML \e segno element.

    TSegno is a visual indicator only.
    It is defined in common.dtd.
*/
class EXP TSegno : public TDirectionTypeElement, public Positionable {
    public:        
        EXP friend SMARTP<TSegno> newSegno();
        virtual void accept(TScoreVisitor& visitor);

    protected:
        TSegno() {}
        virtual ~TSegno() {}
};
typedef SMARTP<TSegno> SSegno;

/*!
\brief Represents the MusicXML \e coda element.

    TCoda is a visual indicator only.
    It is defined in common.dtd.
*/
class EXP TCoda : public TDirectionTypeElement, public Positionable {
    public:
        EXP friend SMARTP<TCoda> newCoda();
        virtual void accept(TScoreVisitor& visitor);

    protected:
        TCoda() {}
        virtual ~TCoda() {}
};
typedef SMARTP<TCoda> SCoda;

/*!
\brief Represents the MusicXML \e dynamics element.

    It is defined in common.dtd.
*/
class EXP TDynamic : public TDirectionTypeElement, public TNotationElement, 
				 public Positionable, public Placementable, public Fontable { 
    public:
        enum dynamic { 	pppppp=1, ppppp, pppp, ppp, pp, p,
				mp, mf,
				f, ff, fff, ffff, fffff, ffffff, 
				sf, sfp, sfpp, fp, rf, rfz, sfz, sffz, fz, 
				last=fz };

        EXP friend SMARTP<TDynamic> newDynamic();

        virtual void accept(TScoreVisitor& visitor);
		vector<dynamic>& dynamics()	{ return fDynamic; }
        long	add (dynamic d);

	//! convert an integer dynamic to a MusicXML string
	static const string	xmldynamic (dynamic d);
	//! convert an MusicXML string to an integer dynamic
	static dynamic		xmldynamic (const string str);
    
    static const string* getDynamicStrings()   { return fDynamicStrings; }
        
	protected:
        TDynamic() {}
        virtual ~TDynamic() {}
        
	private:
        vector<dynamic> fDynamic;

	static bimap<string, dynamic> fDynamic2String;
	static dynamic 		fDynamicTbl[];
	static string 	fDynamicStrings[];
};
typedef SMARTP<TDynamic> SDynamic;

/*!
\brief Represents the MusicXML \e printout entity.

	The printout entity is based on MuseData print
	suggestions. They allow a way to specify not to print
	print an object (e.g. note or rest) or its augmentation
	dots. This is especially useful for notes that overlap
	in different voices. By default, all these attributes
	are set to yes. For wholly invisible notes, such as
	those providing sound-only data, the attribute for
	print-spacing may be set to no so that no space is
	left for this note. The print-spacing value is only
	used if both print-object and print-dot are no.
    It is defined in common.dtd.
*/
class EXP TPrintout : public visitable {
    public:
        TPrintout();
        virtual ~TPrintout() {}

        virtual void accept(TScoreVisitor& visitor);

		void setPrintObject(YesNo::type t)  { fObject = t; }
		void setPrintDot(YesNo::type t)		{ fDot = t; }
		void setPrintSpacing(YesNo::type t) { fSpacing = t; }

		YesNo::type getPrintObject() const  { return fObject; }
		YesNo::type getPrintDot() const		{ return fDot; }
		YesNo::type getPrintSpacing() const { return fSpacing; }

	private:
		YesNo::type fObject;
		YesNo::type fDot;
		YesNo::type fSpacing;
};

/*!
\brief Represents the MusicXML \e bezier entity.

	The bezier entity is used to indicate the curvature of
	slurs. Normal slurs and S-shaped slurs need only two
	bezier points: one associated with the start of the
	slur, the other with the stop. Complex slurs and slurs
	divided over system breaks can specify bezier data at
	slur elements with a continue type. 
	
	The bezier-offset, bezier-x, and bezier-y attributes
	describe the outgoing bezier point for slurs with
	a start type, and the incoming bezier point for
	slurs with types of continue or stop. The attributes
	bezier-offset2, bezier-x2, and bezier-y2 are only
	valid with slurs of type continue, and describe the
	outgoing bezier point.
	
	The bezier-offset and bezier-offset2 attributes are
	measured in terms of musical divisions, like the offset
	element. These are the recommended attributes for
	specifying horizontal position. The other attributes
	are specified in tenths, relative to any position 
	settings associated with the slur element.
    It is defined in common.dtd.
*/
class EXP TBezier : public visitable {
    public:
		enum { undefined = 0x7fff };
		
        TBezier();
        virtual ~TBezier() {}

        virtual void accept(TScoreVisitor& visitor);

		void setOffset(float offset)	{ fOffset = offset; }
		void setOffset2(float offset)   { fOffset2 = offset; }
		void setX(float x)				{ fX = x; }
		void setY(float y)				{ fY = y; }
		void setX2(float x)				{ fX2 = x; }
		void setY2(float y)				{ fY2 = y; }

		float getOffset() const		{ return fOffset; }
		float getOffset2() const	{ return fOffset2; }
		float getX() const			{ return fX; }
		float getY() const			{ return fY; }
		float getX2() const			{ return fX2; }
		float getY2() const			{ return fY2; }

	private:
		float fOffset, fOffset2;
		float fX, fY, fX2, fY2;
};

} // namespace MusicXML

#endif
