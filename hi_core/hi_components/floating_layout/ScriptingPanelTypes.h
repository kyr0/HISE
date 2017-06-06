/*  ===========================================================================
*
*   This file is part of HISE.
*   Copyright 2016 Christoph Hart
*
*   HISE is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   HISE is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with HISE.  If not, see <http://www.gnu.org/licenses/>.
*
*   Commercial licences for using HISE in an closed source project are
*   available on request. Please visit the project's website to get more
*   information about commercial licencing:
*
*   http://www.hartinstruments.net/hise/
*
*   HISE is based on the JUCE library,
*   which also must be licenced for commercial applications:
*
*   http://www.juce.com
*
*   ===========================================================================
*/

#ifndef SCRIPTINGPANELTYPES_H_INCLUDED
#define SCRIPTINGPANELTYPES_H_INCLUDED

class CodeEditorPanel : public PanelWithProcessorConnection

{
public:

	CodeEditorPanel(FloatingTile* parent);;

	~CodeEditorPanel();

	SET_PANEL_NAME("ScriptEditor");

	Component* createContentComponent(int index) override;

	void fillModuleList(StringArray& moduleList) override
	{
		fillModuleListWithType<JavascriptProcessor>(moduleList);
	}

	void contentChanged() override
	{
		refreshIndexList();
	}

	Identifier getProcessorTypeId() const override;

	bool hasSubIndex() const override { return true; }

	void fillIndexList(StringArray& indexList) override;

	void gotoLocation(Processor* p, const String& fileName, int charNumber);

private:

	ScopedPointer<JavascriptTokeniser> tokeniser;
};

class ScriptContentPanel : public PanelWithProcessorConnection,
						   public GlobalScriptCompileListener
{
public:

	struct Canvas;

	class Editor : public Component
	{
	public:

		Editor(Processor* p);

		void resized() override
		{
			viewport->setBounds(getLocalBounds());
		}

	public:

		ScopedPointer<Viewport> viewport;
	};

	void scriptWasCompiled(JavascriptProcessor *processor) override;

	ScriptContentPanel(FloatingTile* parent) :
		PanelWithProcessorConnection(parent)
	{};

	
	SET_PANEL_NAME("ScriptContent");

	Identifier getProcessorTypeId() const override;

	void contentChanged() override
	{
		if (getProcessor() != nullptr)
		{
			getProcessor()->getMainController()->addScriptListener(this, false);
		}
	}

	Component* createContentComponent(int /*index*/) override;

	void fillModuleList(StringArray& moduleList) override
	{
		fillModuleListWithType<JavascriptProcessor>(moduleList);
	}

private:

};



class ScriptWatchTablePanel : public PanelWithProcessorConnection
{
public:

	ScriptWatchTablePanel(FloatingTile* parent) :
		PanelWithProcessorConnection(parent)
	{
		
	};

	SET_PANEL_NAME("ScriptWatchTable");

	Identifier getProcessorTypeId() const override;

	Component* createContentComponent(int /*index*/) override;

	void fillModuleList(StringArray& moduleList) override
	{
		fillModuleListWithType<JavascriptProcessor>(moduleList);
	}

private:

	const Identifier showConnectionBar;

};

class ConnectorHelpers
{
public:
    
    static void tut(PanelWithProcessorConnection* connector, const Identifier &idToSearch);
    
private:
    
    
};

template <class ProcessorType> class GlobalConnectorPanel : public PanelWithProcessorConnection
{
public:


	GlobalConnectorPanel(FloatingTile* parent) :
		PanelWithProcessorConnection(parent)
	{

	}

	Identifier getIdentifierForBaseClass() const override
	{
		return GlobalConnectorPanel<ProcessorType>::getPanelId();
	}

	static Identifier getPanelId()
	{
		String n;

		n << "GlobalConnector" << ProcessorType::getConnectorId().toString();

		return Identifier(n);
	}

	int getFixedHeight() const override { return 18; }

	Identifier getProcessorTypeId() const override
	{
		RETURN_STATIC_IDENTIFIER("Skip");
	}

	bool showTitleInPresentationMode() const override
	{
		return false;
	}

	bool hasSubIndex() const override { return false; }

	Component* createContentComponent(int /*index*/) override
	{
		return new Component();
	}

    void contentChanged() override
	{
        Identifier idToSearch = ProcessorType::getConnectorId();
        
        ConnectorHelpers::tut(this, idToSearch);
        
	}

	void fillModuleList(StringArray& moduleList) override
	{
		fillModuleListWithType<ProcessorType>(moduleList);
	};

private:

};


class ConsolePanel : public FloatingTileContent,
	public Component
{
public:

	SET_PANEL_NAME("Console");

	ConsolePanel(FloatingTile* parent);

	void resized() override;

	Console* getConsole() const { return console; }

	

private:

	ScopedPointer<Console> console;

};

struct BackendCommandIcons
{
	static Path getIcon(int commandId);
};



#endif  // SCRIPTINGPANELTYPES_H_INCLUDED
