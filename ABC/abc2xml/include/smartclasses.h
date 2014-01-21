/*

  MusicXML Library
  Copyright (C) 2003  Grame

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation> either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY> without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library> if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  Grame Research Laboratory, 9 rue du Garet, 69001 Lyon - France
  grame@grame.fr

*/

#ifndef __smartclasses__
#define __smartclasses__

#include "smartpointer.h"

namespace MusicXML 
{
template <class C> class SmartVisitable;

	typedef SMARTP<class TAccidental> SAccidental;
	typedef SMARTP<class TAccidentalMark> SAccidentalMark;
	typedef SMARTP<class TArpeggiate> SArpeggiate;
	typedef SMARTP<class TNonArpeggiate> SNonArpeggiate;
	typedef SMARTP<class TArticulationElement> SArticulationElement;
	typedef SMARTP<class TArticulations> SArticulations;
	typedef SMARTP<class TAttributes> SAttributes;
	typedef SMARTP<class TBackup> SBackup;
	typedef SMARTP<class TBarline> SBarline;
	typedef SMARTP<class TBass> SBass;
	typedef SMARTP<class TBeam> SBeam;
	typedef SMARTP<class TBeatRepeat> SBeatRepeat;
	typedef SMARTP<class TBend> SBend;
	typedef SMARTP<class TBendWithBar> SBendWithBar;
	typedef SMARTP<class TBracket> SBracket;
	typedef SMARTP<class TChord> SChord;
	typedef SMARTP<class TClef> SClef;
	typedef SMARTP<class TCoda> SCoda;
	typedef SMARTP<class TCreator> SCreator;
	typedef SMARTP<class TDashes> SDashes;
	typedef SMARTP<class TDegree> SDegree;
	typedef SMARTP<class TDirection> SDirection;
	typedef SMARTP<class TDirectionType> SDirectionType;
	typedef SMARTP<class TDirective> SDirective;
	typedef SMARTP<class TDynamic> SDynamic;
	typedef SMARTP<class TEncoding> SEncoding;
	typedef SMARTP<class TEnding> SEnding;
	typedef SMARTP<class TExpression> SExpression;
	typedef SMARTP<class TFermata> SFermata;
	typedef SMARTP<class TFigure> SFigure;
	typedef SMARTP<class TFingering> SFingering;
	typedef SMARTP<class TFeature> SFeature;
	typedef SMARTP<class TFiguredBass> SFiguredBass;
	typedef SMARTP<class TFootnote> SFootnote;
	typedef SMARTP<class TForward> SForward;
	typedef SMARTP<class TFrame> SFrame;
	typedef SMARTP<class TFrameNote> SFrameNote;
	typedef SMARTP<class TFunction> SFunction;
	typedef SMARTP<class TGlissando> SGlissando;
	typedef SMARTP<class TGraceNote> SGraceNote;
	typedef SMARTP<class TGraphNote> SGraphNote;
	typedef SMARTP<class TGrouping> SGrouping;
	typedef SMARTP<class THammerPull> SHammerPull;
	typedef SMARTP<class THarmonic> SHarmonic;
	typedef SMARTP<class THarmony> SHarmony;
	typedef SMARTP<class THarmonyChord> SHarmonyChord;
	typedef SMARTP<class TIdentification> SIdentification;
	typedef SMARTP<class TKey> SKey;
	typedef SMARTP<class TLyric> SLyric;
	typedef SMARTP<class TLevel> SLevel;
	typedef SMARTP<class TMeasure> SMeasure;
	typedef SMARTP<class TMeasureRepeat> SMeasureRepeat;
	typedef SMARTP<class TMeasureStyle> SMeasureStyle;
	typedef SMARTP<class TMeasureStyleElement> SMeasureStyleElement;
	typedef SMARTP<class TMetronome> SMetronome;
	typedef SMARTP<class TMidiDevice> SMidiDevice;
	typedef SMARTP<class TMidiInstrument> SMidiInstrument;
	typedef SMARTP<class TMiscField> SMiscField;
	typedef SMARTP<class TMiscId> SMiscId;
	typedef SMARTP<class TMordentOrnament> SMordentOrnament;
	typedef SMARTP<class TMultipleLyricPart> SMultipleLyricPart;
	typedef SMARTP<class TMultipleRest> SMultipleRest;
	typedef SMARTP<class TMusicXMLReader> SMusicXMLReader;
	typedef SMARTP<class TNotation> SNotation;
	typedef SMARTP<class TNote> SNote;
	typedef SMARTP<class TNoteHead> SNoteHead;
	typedef SMARTP<class TOctaveShift> SOctaveShift;
	typedef SMARTP<class TOrnament> SOrnament;
	typedef SMARTP<class TOrnaments> SOrnaments;
	typedef SMARTP<class TOtherArticulation> SOtherArticulation;
	typedef SMARTP<class TOtherDirection> SOtherDirection;
	typedef SMARTP<class TOtherDirections> SOtherDirections;
	typedef SMARTP<class TOtherNotation> SOtherNotation;
	typedef SMARTP<class TOtherOrnament> SOtherOrnament;
	typedef SMARTP<class TPWMeasure> SPWMeasure;
	typedef SMARTP<class TPWPart> SPWPart;
	typedef SMARTP<class TPart> SPart;
	typedef SMARTP<class TPartGroup> SPartGroup;
	typedef SMARTP<class TPartList> SPartList;
	typedef SMARTP<class TPedal> SPedal;
	typedef SMARTP<class TPCData> SPCData;
	typedef SMARTP<class TPitch> SPitch;
	typedef SMARTP<class TPrint> SPrint;
	typedef SMARTP<class TRehearsal> SRehearsal;
	typedef SMARTP<class TRepeat> SRepeat;
	typedef SMARTP<class TRest> SRest;
	typedef SMARTP<class TRights> SRights;
	typedef SMARTP<class TRoot> SRoot;
	typedef SMARTP<class TScore> SScore;
	typedef SMARTP<class TScoreHeader> SScoreHeader;
	typedef SMARTP<class TScoreInstrument> SScoreInstrument;
	typedef SMARTP<class TScorePart> SScorePart;
	typedef SMARTP<class TScorePartwise> SScorePartwise;
	typedef SMARTP<class TScoreTimewise> SScoreTimewise;
	typedef SMARTP<class TSegno> SSegno;
	typedef SMARTP<class TSlash> SSlash;
	typedef SMARTP<class TSlide> SSlide;
	typedef SMARTP<class TSlur> SSlur;
	typedef SMARTP<class TStaffTuning> SStaffTuning;
	typedef SMARTP<class TStaffDetails> SStaffDetails;
	typedef SMARTP<class TSupport> SSupport;
	typedef SMARTP<class TSound> SSound;
	typedef SMARTP<class TStrongAccent> SStrongAccent;
	typedef SMARTP<class TTWMeasure> STWMeasure;
	typedef SMARTP<class TTWPart> STWPart;
	typedef SMARTP<class TTechnical> STechnical;
	typedef SMARTP<class TTechnicals> STechnicals;
	typedef SMARTP<class TTie> STie;
	typedef SMARTP<class TTieSlur> STieSlur;
	typedef SMARTP<class TTimeModification> STimeModification;
	typedef SMARTP<class TTimeSign> STimeSign;
	typedef SMARTP<class TTranspose> STranspose;
	typedef SMARTP<class TTrillOrnament> STrillOrnament;
	typedef SMARTP<class TTuplet> STuplet;
	typedef SMARTP<class TTupletDesc> STupletDesc;
	typedef SMARTP<class TUnpitched> SUnpitched;
	typedef SMARTP<class TWavyLine> SWavyLine;
	typedef SMARTP<class TVoice> SVoice;
	typedef SMARTP<class TWedge> SWedge;
	typedef SMARTP<class TWords> SWords;
	typedef SMARTP<class TWork> SWork;

class TOrientation;
class TPlacement;
class TPosition;
class TTrillSound;
class TBendSound;
class TFont;
class TPrintout;
class TEditorial;
class TBezier;

}

#endif

