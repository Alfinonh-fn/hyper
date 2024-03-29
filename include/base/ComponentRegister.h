#pragma once

#include <utils/Components.h>
#include <utils/Logger.h>

// STL includes
#include <map>

#include <QObject>

class HyperHdrInstance;

///
/// @brief The component register reflects and manages the current state of all components and HyperHDR as a whole
/// It emits also real component state changes (triggert from the specific component), which can be used for listening APIs (Network Clients/Plugins)
///
class ComponentRegister : public QObject
{
	Q_OBJECT

public:
	ComponentRegister(HyperHdrInstance* hyperhdr);
	~ComponentRegister() override;

	///
	/// @brief  Check if a component is currently enabled
	/// @param  comp   The component from enum
	/// @return        True if component is running else false. Not found is -1
	///
	int isComponentEnabled(hyperhdr::Components comp) const;

	/// contains all components and their state
	std::map<hyperhdr::Components, bool> getRegister() const { return _componentStates; }

signals:
	///
	///	@brief Emits whenever a component changed (really) the state
	///	@param comp   The component
	///	@param state  The new state of the component
	///
	void updatedComponentState(hyperhdr::Components comp, bool state);

public slots:
	///
	/// @brief is called whenever a component change a state, DO NOT CALL FROM API, use signal hyperhdr->compStateChangeRequest
	///	@param comp   The component
	///	@param state  The new state of the component
	///
	void setNewComponentState(hyperhdr::Components comp, bool activated);

private slots:
	///
	/// @brief Handle COMP_ALL changes from Hyperhdr->compStateChangeRequest
	///
	void handleCompStateChangeRequest(hyperhdr::Components comps, bool activated);

private:
	///  HyperHDR instance
	HyperHdrInstance* _hyperhdr;
	/// Logger instance
	Logger* _log;
	/// current state of all components
	std::map<hyperhdr::Components, bool> _componentStates;
	/// on hyperHDR off we save the previous states of all components
	std::map<hyperhdr::Components, bool> _prevComponentStates;
	// helper to prevent self emit chains
	bool _inProgress = false;
};
