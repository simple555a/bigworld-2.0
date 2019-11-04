/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CHAIN_EDITORS_HPP
#define CHAIN_EDITORS_HPP


#include "effect_node.hpp"
#include "phase_node.hpp"


/**
 *	Base interface class for python sequence editing.
 */
class SequenceEditor : public ReferenceCount
{
public:
	virtual bool isOK() const = 0;
	virtual void modify( PyObjectPtr pSeq, int i, PyObjectPtr pNewSeq ) = 0;
};
typedef SmartPointer< SequenceEditor > SequenceEditorPtr;


/**
 *	Add an effect to the chain.
 */
class AddEffectEditor : public SequenceEditor
{
public:
	AddEffectEditor( const std::wstring & effectName, EffectNodePtr beforeNode );
	AddEffectEditor( PyObjectPtr pyEffect, EffectNodePtr beforeNode );
	bool isOK() const;
	void modify( PyObjectPtr pChain, int i, PyObjectPtr pNewChain );

private:
	PyObjectPtr pyNewEffect_;
	EffectNodePtr beforeNode_;
};


/**
 *	Delete an effect from the chain.
 */
class DeleteEffectEditor : public SequenceEditor
{
public:
	DeleteEffectEditor( EffectNodePtr deleteNode /* NULL for delete all */ );
	bool isOK() const;
	void modify( PyObjectPtr pChain, int i, PyObjectPtr pNewChain );

private:
	EffectNodePtr deleteNode_;
};


/**
 *	Move an effect to another part of the chain.
 */
class MoveEffectEditor : public SequenceEditor
{
public:
	MoveEffectEditor( EffectNodePtr moveNode, EffectNodePtr beforeNode );
	bool isOK() const;
	void modify( PyObjectPtr pChain, int i, PyObjectPtr pNewChain );

private:
	EffectNodePtr moveNode_;
	EffectNodePtr beforeNode_;
};


/**
 *	Add a phase to an effect.
 */
class AddPhaseEditor : public SequenceEditor
{
public:
	AddPhaseEditor( const std::wstring & phaseName, PhaseNodePtr beforePhase, PyObjectPtr pEditorPhasesModule );
	AddPhaseEditor( PyObjectPtr pyPhase, PhaseNodePtr beforePhase );
	bool isOK() const;
	void modify( PyObjectPtr pPhases, int i, PyObjectPtr pNewPhases );

private:
	PyObjectPtr pyNewPhase_;
	PhaseNodePtr beforePhase_;
};


/**
 *	Delete a phase from an effect.
 */
class DeletePhaseEditor : public SequenceEditor
{
public:
	DeletePhaseEditor( PhaseNodePtr deleteNode );
	bool isOK() const;
	void modify( PyObjectPtr pPhases, int i, PyObjectPtr pNewPhases );

private:
	PhaseNodePtr deleteNode_;
};


#endif // CHAIN_EDITORS_HPP
