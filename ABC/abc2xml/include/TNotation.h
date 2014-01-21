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

#ifndef __TNotation__
#define __TNotation__

#include "exports.h"
#include "common.h"
#include "smartpointer.h"
#include "TScoreVisitor.h"

namespace MusicXML 
{

/*!
\brief Represents musical notations.

	Multiple notations are allowed in order to represent
	multiple editorial levels. The set of notations will be
	refined and expanded over time, especially to handle
	more instrument-specific technical notations.
\n  
    TNotation corresponds to the MusicXML \e notation element
    as defined in note.dtd.
*/
class EXP TNotation : public Editorialable, public visitable, public smartable {

    public:
        EXP friend SMARTP<TNotation> newNotation();
        
        virtual void accept(TScoreVisitor& visitor);

		long add (const SNotationElement& elt);
        vvector<SNotationElement>& list()			{ return fList; }

    protected:
        TNotation() {}
        virtual ~TNotation() {}
    private:
        vvector<SNotationElement>	fList;
};
typedef SMARTP<TNotation> SNotation;

/*!
\brief base class for ties and slurs

	Ties and slurs share common attributes and are represented by a single object
	derived into TTie and TSlur.
	
*/
class TTieSlur : public TNotationElement, public Positionable, public Placementable, public Orientable {

    public:
        enum { undefined=-1 };
        
        virtual void accept(TScoreVisitor& visitor) = 0;

        StartStop::type	getType() const			{ return fType; }
        int				getNumber() const		{ return fNumber; }
        LineType::type	getLineType() const		{ return fLine; }

        void	setType(StartStop::type type);
        void 	setNumber(int num);
        void	setLineType(LineType::type type);

        const string& getElement() const			{ return fElement; }

    protected:
        TTieSlur(string elt, StartStop::type type) 
            : fElement(elt), fType(type), fNumber(undefined), fLine(LineType::undefined) {}
        virtual ~TTieSlur() {}
    private:
        string 			fElement;
        StartStop::type	fType;
        int				fNumber;
        LineType::type	fLine;
 };
 typedef SMARTP<TTieSlur> STieSlur;
 
/*!
\brief Ties representation.

    MusicXML \e ties are defined in note.dtd.
*/
class EXP TTie : public TTieSlur {
    public:
		EXP friend SMARTP<TTie> newTie(StartStop::type type = StartStop::undefined);
        virtual void accept(TScoreVisitor& visitor);
    protected:
        TTie(StartStop::type type) : TTieSlur("tied", type) {} 
        virtual ~TTie() {}
 };
 typedef SMARTP<TTie> STie;

/*!
\brief Slur representation.

    MusicXML \e slur are defined in note.dtd.
*/
class EXP TSlur : public TTieSlur {
    public:
		EXP friend SMARTP<TSlur> newSlur(StartStop::type type = StartStop::undefined);
        virtual void accept(TScoreVisitor& visitor);
		TBezier& bezier()   { return fBezier; }

    protected:
        TSlur(StartStop::type type) : TTieSlur("slur", type) {} 
        virtual ~TSlur() {}
	private:
		TBezier fBezier;
 };
 typedef SMARTP<TSlur> SSlur;

/*!
\brief a tuplet description.

    It correspond to the \e tuplet-actual and \e tuplet-normal
    MusicXML elements as defined in note.dtd.
*/
class EXP TTupletDesc : public visitable, public smartable {

    public:
        enum { undefined=-1 };

        virtual void accept(TScoreVisitor& visitor);

        void	setNumber (int num);
        void	setType(int type);
        void	setDots(int dots);

        int		getNumber() const		{ return fNumber; }
        int		getType() const			{ return fType; }
        int		getDots() const			{ return fDots; }

        const string getName() const	{ return fName; }

	protected:
        TTupletDesc(string suffix) 
            : fName(suffix), fNumber(undefined), fType(undefined), fDots(0) {}
        virtual ~TTupletDesc() {}

    private:
        //! the MusicXML element suffix name (should be "actual" or "normal")
        string 	fName;
        int		fNumber;
        int		fType;
        int		fDots;
};
typedef SMARTP<TTupletDesc> STupletDesc;

/*!
\brief an actual tuplet description.

    It correspond to the \e tuplet-actual MusicXML elements as defined in note.dtd.
*/
class EXP TTupletActual: public TTupletDesc {
    public:
        EXP friend STupletDesc newTupletActual();
	protected:
        TTupletActual() : TTupletDesc("actual") {}
        virtual ~TTupletActual() {}
};

/*!
\brief a normal tuplet description.

    It correspond to the \e tuplet-normal MusicXML elements as defined in note.dtd.
*/
class EXP TTupletNormal: public TTupletDesc {
    public:
        EXP friend STupletDesc newTupletNormal();
	protected:
        TTupletNormal() : TTupletDesc("normal") {}
        virtual ~TTupletNormal() {}
};

/*!
\brief Graphical representation of a tuplet.

	A tuplet element is present when a tuplet is to be
	displayed graphically, in addition to the sound data
	provided by the time-modification elements. The number
	attribute is used to distinguish nested tuplets. The
	bracket attribute is used to indicate the presence of a
	bracket. If unspecified, the results are implementation-
	dependent. 
	
	The tuplet-actual and tuplet-normal elements provide optional full control
	over tuplet specifications. Each allows the number
	and note type (including dots) describing a single
	tuplet. If any of these elements are absent, their
	values are based on the time-modification element.
	
	The show-number attribute is used to display either
	the number of actual notes, the number of both actual
	and normal notes, or neither. It is actual by default.
	The show-type attribute is used to display either the
	actual type, both the actual and normal types, or
	neither. It is none by default.
*/         
class EXP TTuplet : public TNotationElement, public Placementable, public Positionable {

    public:
        enum { undefined=-1, actual=1, both, none, last=none };
        
		EXP friend SMARTP<TTuplet> newTuplet(StartStop::type type = StartStop::undefined);
        virtual void accept(TScoreVisitor& visitor);
        
        //! the required type attribute
        void 	setType(StartStop::type type);
        //! optional number for nested tuplets
        void	setNumber(int num);
        //! optional bracket attribute
        void 	setBracket(YesNo::type bracket);
        //! optional display control
        void	setShowNum(int sn);
        //! optional display control
        void	setShowType(int st);

        StartStop::type getType() const	{ return fType; }
        int		getNumber() const		{ return fNumber; }
        YesNo::type getBracket() const		{ return fBracket; }
        int		getShowNum() const		{ return fShowNum; }
        int		getShowType() const		{ return fShowType; }

        STupletDesc&	actualDesc()	{ return fActual; }
        STupletDesc&	normalDesc() 	{ return fNormal; }

	//! convert a numeric show type to a MusicXML string
	static const string	xmlshow (int d);
	//! convert an MusicXML string to a numeric show type
	static int			xmlshow (const string str);

    protected:
        TTuplet(StartStop::type type) 
            : fType(type), fNumber(undefined), fBracket(YesNo::undefined),
              fShowNum(undefined), fShowType(undefined) {}
        virtual ~TTuplet() {}

    private:
        StartStop::type	fType;
        int		fNumber;
        YesNo::type 	fBracket;
        int		fShowNum;
        int		fShowType;

        STupletDesc	fActual;
        STupletDesc	fNormal;

	static bimap<string, int> fShow2String;
	static int 		fShowTbl[];
	static string 	fShowStrings[];
};
typedef SMARTP<TTuplet> STuplet;

/*!
\brief glissando representation

	Glissando and slide elements both indicate rapidly
	moving from one pitch to the other so that individual
	notes are not discerned. The distinction is similar
	to that between NIFF's glissando and portamento 
	elements. A glissando sounds the half notes in
	between the slide and defaults to a wavy line. A
	slide is continuous between two notes and defaults
	to a solid line.
*/
class EXP TGlissando : public TNotationElement, public Placementable, public Positionable {
    public:
        enum { undefined=-1, actual=1, both, none, last=none };
        
		EXP friend SMARTP<TGlissando> newGlissando(StartStop::type type = StartStop::undefined);
        virtual void accept(TScoreVisitor& visitor);
        
        //! the required type attribute
        void 	setType(StartStop::type type);
        void 	setNumber(int number);
        void 	setLineType(LineType::type type);

        StartStop::type getType() const			{ return fType; }
        int				getNumber() const		{ return fNumber; }
        LineType::type 	getLineType() const		{ return fLineType; }

    protected:
        TGlissando(StartStop::type type) 
            : fType(type), fNumber(1), fLineType(LineType::undefined) {}
        virtual ~TGlissando() {}

	private:
        StartStop::type	fType;
        int				fNumber;
        LineType::type	fLineType;
};
typedef SMARTP<TGlissando> SGlissando;

/*!
\brief slide representation
*/
class EXP TSlide : public TGlissando, public Bendable {
    public:
		EXP friend SMARTP<TSlide> newSlide (StartStop::type type = StartStop::undefined);
        virtual void accept(TScoreVisitor& visitor);

    protected:
        TSlide(StartStop::type type) : TGlissando(type) {}
        virtual ~TSlide() {}
};
typedef SMARTP<TSlide> SSlide;

/*!
\brief articulations and accents definition.

    Articulations elements are all handled by a single object
	with a type field used to specify the articulation type.
	All the articulations have a position and placement attributes.
    Apart the \e other-articulation element, all are empty elements.
\n  TArticulation handles a set of elements defined in note.dtd.

\todo support of the line-shape and line-type attributes
*/
class EXP TArticulationElement : public visitable, public Positionable, public Placementable, public smartable {

    public:
        enum articulation { first=1, accent=first, strong_accent, staccato, tenuto, detached_legato,
             staccatissimo, spiccato, scoop, plop, doit, falloff, breath_mark, caesura, 
			 other_articulation, last=other_articulation };

        EXP friend SMARTP<TArticulationElement> newArticulationElement(articulation type);
        virtual void accept(TScoreVisitor& visitor);

        articulation 	getArtType() const			{ return fArticulationType; }
        void 			setArtType(articulation a);
		
	//! convert a numeric articulation name to a MusicXML string
	static const string	xmlname (articulation d);
	//! convert an MusicXML string to a numeric articulation name
	static articulation	xmlname (const string str);
    
    static const string* getArticulationStrings()   { return fArticulationStrings; }
        
    protected:
        TArticulationElement(articulation type) : fArticulationType(type) {}
        virtual ~TArticulationElement() {}

    private:
        articulation 	fArticulationType;

	static bimap<string, articulation> fArticulation2String;
	static articulation 		fArticulationTbl[];
	static string 				fArticulationStrings[];
};
typedef SMARTP<TArticulationElement> SArticulationElement;

/*!
\brief other articulations
*/
class EXP TOtherArticulation : public TArticulationElement {
    public:
        EXP friend SMARTP<TOtherArticulation> newOtherArticulation();
        virtual void accept(TScoreVisitor& visitor);
		void			setData(const string& data)		{ fData = data; }
		const string&   getData() const					{ return fData; }
    protected:
        TOtherArticulation() : TArticulationElement(other_articulation) {}
        virtual ~TOtherArticulation() {}
	private:
		string  fData;
};
typedef SMARTP<TOtherArticulation> SOtherArticulation;

/*!
\brief The MusicXML \e articulations element.

    Defined in note.dtd.
*/
class EXP TArticulations : public TNotationElement {

      public:
        EXP friend SMARTP<TArticulations> newArticulations();
        virtual void accept(TScoreVisitor& visitor);

        long add (const SArticulationElement& art);
        vvector<SArticulationElement>& articulations()	{ return fList; }

    protected:
        TArticulations() {}
        virtual ~TArticulations() {}
    private:
        vvector<SArticulationElement>	fList;
};
typedef SMARTP<TArticulations> SArticulations;

/*!
\brief a strong accent representation.

    a strong accent is an articulation but have an additional type attributes.
*/
class EXP TStrongAccent : public TArticulationElement {

    public:
        enum { undefined, up=1, down, last=down };
        EXP friend SMARTP<TStrongAccent> newStrongAccent();
        virtual void accept(TScoreVisitor& visitor);

        int		getType() const		{ return fType; }
        void 	setType(int type);
        
	//! convert a numeric accent type to a MusicXML string
	static const string	xmltype (int d);
	//! convert an MusicXML string to a numeric accent type
	static int			xmltype (const string str);

    protected:
        TStrongAccent() : TArticulationElement(strong_accent), fType(undefined) {}
        virtual ~TStrongAccent() {}
    private:
        int 	fType;

	static bimap<string, int> fType2String;
	static int 		fTypeTbl[];
	static string 	fTypeStrings[];
};
typedef SMARTP<TStrongAccent> SStrongAccent;


/*!
\brief other notations representation

	The other-notation element is used to define
	any notations not yet in MusicXML. This allows 
	extended representation, though without application
	interoperability. It handles notations where more 
	specific extension elements such as other-dynamics 
	and other-technical are not appropriate.
*/
class EXP TOtherNotation : public TNotationElement, public Placementable, public Positionable {
    public:
        enum { undefined=-1, actual=1, both, none, last=none };
        
		EXP friend SMARTP<TOtherNotation> newOtherNotation(StartStop::type type = StartStop::undefined);
        virtual void accept(TScoreVisitor& visitor);
        
        //! the required type attribute
        void 	setType(StartStop::type type);
        void 	setNumber(int number);

        StartStop::type getType() const			{ return fType; }
        int				getNumber() const		{ return fNumber; }

    protected:
        TOtherNotation(StartStop::type type) : fType(type), fNumber(1) {}
        virtual ~TOtherNotation() {}

	private:
        StartStop::type	fType;
        int				fNumber;
};
typedef SMARTP<TOtherNotation> SOtherNotation;

} // namespace MusicXML


#endif
