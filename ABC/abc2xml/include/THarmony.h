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

#ifndef __THarmony__
#define __THarmony__

#include "exports.h"
#include "common.h"
#include "smartpointer.h"
#include "TScoreVisitor.h"
#include "TTechnical.h"

using namespace std;

namespace MusicXML 
{

/*!
\brief base class for the root and function elements of the \e harmony-chord entity.
*/
class TRootFunction : public virtual visitable, public virtual smartable {
     protected:
        TRootFunction () {}
        virtual ~TRootFunction() {}
};
typedef SMARTP<TRootFunction> 	SRootFunction;

/*!
\brief the function element of the \e harmony-chord entity.

	A  function is an indication like I, II, III. Function is generally used
	with classical functional harmony. . Function requires that the key be
	specified in the encoding. 
*/
class TFunction : public TRootFunction {
    public:
        EXP friend SMARTP<TFunction> newFunction();        
		virtual void accept(TScoreVisitor& visitor);
        void	setFunction(const std::string& function);
        const std::string&		getFunction() const		{ return fFunction; }
	protected:
        TFunction () {}
        virtual ~TFunction() {}
    private:
		std::string fFunction;
};
typedef SMARTP<TFunction> 	SFunction;

/*!
\brief the root element of the \e harmony-chord entity.

	A root is a pitch name like C, D, E. Root is generally used
	with pop chord symbols, function with classical
	functional harmony. The root element has a
	root-step and optional root-alter similar to the step
	and alter elements in a pitch.
*/
class TRoot : public TRootFunction {
    public:
        EXP friend SMARTP<TRoot> newRoot();        
		virtual void accept(TScoreVisitor& visitor);
		
        void	setRoot(const std::string& root);
        void	setAlter(const std::string& alter);
        const std::string&		getRoot() const		{ return fRoot; }
        const std::string&		getAlter() const	{ return fAlter; }
     protected:
        TRoot () {}
        virtual ~TRoot() {}
    private:
		std::string fRoot;
		std::string fAlter;
};
typedef SMARTP<TRoot> 	SRoot;

/*!
\brief the bass element of the \e harmony-chord entity.

	Bass is used to indicate a bass note in popular music
	chord symbols, e.g. G/C. It is generally not used in
	functional harmony, as inversion is generally not used
	in pop chord symbols. As with root, it is divided into
	step and alter elements, similar to pitches.
*/

class TBass : public TRoot {
    public:
        EXP friend SMARTP<TBass> newBass();  
		virtual void accept(TScoreVisitor& visitor);      
};
typedef SMARTP<TBass> 	SBass;

/*!
\brief the \e degree element of the \e harmony-chord entity.

	The degree element is used to add, alter, or subtract
	individual notes in the chord. The degree-value element
	is a number indicating the degree of the chord (1 for
	the root, 3 for third, etc). The degree-alter element
	is like the alter element in notes: 1 for sharp, -1 for
	flat, etc. The degree-type element can be add, alter, or
	subtract. If the degree-type is alter or subtract, the
	degree-alter is relative to the degree already in the
	chord based on its kind element. If the degree-type is
	add, the degree-alter is relative to a dominant chord
	(major and perfect intervals except for a minor 
	seventh).
	
	A harmony of kind "other" can be spelled explicitly by
	using a series of degree elements together with a root.
*/
class TDegree : public smartable, public visitable {
    public:
        EXP friend SMARTP<TDegree> newDegree();        
		virtual void accept(TScoreVisitor& visitor);
		
        void	setValue(int val);
        void	setAlter(int val);
        void	setType(const std::string& alter);
        int					getValue() const	{ return fValue; }
        int					getAlter() const	{ return fAlter; }
        const std::string&  getType() const		{ return fType; }
     protected:
        TDegree () : fValue(0), fAlter(0) {}
        virtual ~TDegree() {}
    private:
		int fValue, fAlter;
		std::string fType;
};
typedef SMARTP<TDegree> 	SDegree;

/*! the \e harmony-chord entity part of the \e harmony element.
*/
class EXP THarmonyChord : public smartable, public visitable {
    public:
		enum { undefined=-1 };
        EXP friend SMARTP<THarmonyChord> newHarmonyChord();
        
		virtual void accept(TScoreVisitor& visitor);

        void	setKind(const std::string kind);
        void	setInversion(int n);
		void	setRootFunction(SRootFunction function);
		void	setBass(SBass bass);
        void	add(SDegree degree);

        const std::string&		getKind() const			{ return fKind; }
        int						getInversion() const	{ return fInversion; }

        SRootFunction		getRootFunction() const		{ return fRootFunction; }
        SBass				getBass() const				{ return fBass; }
		vvector<SDegree>&   getDegrees()				{ return fDegrees; }

    protected:
        THarmonyChord() : fInversion(undefined) {}
        virtual ~THarmonyChord() {}
    private:
		SRootFunction   fRootFunction;
		std::string		fKind;
		int				fInversion;
		SBass			fBass;
		vvector<SDegree> fDegrees;
};
typedef SMARTP<THarmonyChord> SHarmonyChord;

/*! a frame element used in the \e harmony element.
*/
class EXP TFrame : public smartable, public visitable {
    public:
		enum { undefined };
        EXP friend SMARTP<TFrame> newFrame();
        
		virtual void accept(TScoreVisitor& visitor);

        void	setStrings(int n);
        void	setFrets(int n);
        void	setFirstFret(int n);
        int		getStrings() const		{ return fStrings; }
        int		getFrets() const		{ return fFrets; }
        int		getFirstFret() const	{ return fFirstFret; }

		void	add(SFrameNote fn);
		vvector<SFrameNote>& frameNotes()   { return fFrameNotes; }
 
    protected:
        TFrame() : fStrings(undefined), fFrets(undefined), fFirstFret(undefined) {}
        virtual ~TFrame() {}
    private:
		int fStrings, fFrets, fFirstFret;
		vvector<SFrameNote> fFrameNotes;
};
typedef SMARTP<TFrame> SFrame;

/*! a frame-note used in the \e harmony element.

\todo : use the TString and TFret classes (TTechnical.h)
*/
class EXP TFrameNote : public smartable, public visitable {
    public:
        EXP friend SMARTP<TFrameNote> newFrameNote();
        
		virtual void accept(TScoreVisitor& visitor);

        void	setString(const std::string str);
        void	setFret(const std::string str);
        const std::string& 	getString() const   { return fString; }
        const std::string& 	getFret() const		{ return fFret; }
		SFingering& fingering()					{ return fFingering; }
 
    protected:
        TFrameNote() {}
        virtual ~TFrameNote() {}
    private:
		std::string fString;
		std::string fFret;
		SFingering  fFingering;
};
typedef SMARTP<TFrameNote> SFrameNote;

/*! the MusicXML \e harmony element.
*/
class EXP THarmony : public TMusicData, public Positionable, public Placementable, public Editorialable {
    public:
		enum type { undefined, kexplicit, implied, alternate, last=alternate };
		
        EXP friend SMARTP<THarmony> newHarmony();
        
		virtual void accept(TScoreVisitor& visitor);

        void	setType(type t);
        void 	setPrintObject(YesNo::type yn);
        void 	setPrintFrame(YesNo::type yn);
        void 	add(SHarmonyChord hc);

        type			getType() const			{ return fType; }
        YesNo::type 	getPrintObject() const	{ return fPrintObject; }
        YesNo::type 	getPrintFrame() const   { return fPrintFrame; }

		SFrame&			frame()					{ return fFrame; }
		vvector<SHarmonyChord>& getHarmonyChords() { return fHarmonyChord; }

	static const string		xml (type t);
	static 	type			xml (const string str);

    protected:
        THarmony() : fType(undefined), fPrintObject(YesNo::undefined), fPrintFrame(YesNo::undefined) {}
        virtual ~THarmony() {}
    private:
		type	fType;
		YesNo::type fPrintObject;
		YesNo::type fPrintFrame;
		SFrame		fFrame;
		vvector<SHarmonyChord> fHarmonyChord;

	static bimap<string, type> fType2String;
	static type 		fTypeTbl[];
	static string		fTypeStrings[];
};
typedef SMARTP<THarmony> SHarmony;


} // namespace MusicXML


#endif
