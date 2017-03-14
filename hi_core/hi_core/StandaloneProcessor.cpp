
AudioDeviceDialog::AudioDeviceDialog(AudioProcessorDriver *ownerProcessor_) :
ownerProcessor(ownerProcessor_)
{
	setOpaque(true);

	selector = new AudioDeviceSelectorComponent(*ownerProcessor->deviceManager, 0, 0, 2, 2, true, false, true, false);

	setLookAndFeel(&alaf);

	selector->setLookAndFeel(&pplaf);

	addAndMakeVisible(cancelButton = new TextButton("Cancel"));
	addAndMakeVisible(applyAndCloseButton = new TextButton("Apply changes & close window"));

	cancelButton->addListener(this);
	applyAndCloseButton->addListener(this);

	addAndMakeVisible(selector);
}


void AudioDeviceDialog::buttonClicked(Button *b)
{
	if (b == applyAndCloseButton)
	{
		ownerProcessor->saveDeviceSettingsAsXml();
		ScopedPointer<XmlElement> deviceData = ownerProcessor->deviceManager->createStateXml();
		ownerProcessor->initialiseAudioDriver(deviceData);
	}

#if USE_BACKEND
	dynamic_cast<BackendProcessorEditor*>(getParentComponent())->showSettingsWindow();
#endif
}

File AudioProcessorDriver::getDeviceSettingsFile()
{

	File parent = getSettingDirectory();

	File savedDeviceData = parent.getChildFile("DeviceSettings.xml");

	return savedDeviceData;
}


void AudioProcessorDriver::restoreSettings(MainController* mc)
{
	ScopedPointer<XmlElement> deviceData = getSettings();

	if (deviceData != nullptr)
	{
		

#if HISE_IOS
		deviceData->setAttribute("audioDeviceBufferSize", 512);
#endif

		
	}
    
#if USE_FRONTEND
    bool allSamplesThere = deviceData != nullptr && deviceData->getBoolAttribute("SAMPLES_FOUND");
    
    if (!allSamplesThere)
    {
        dynamic_cast<FrontendProcessor*>(mc)->checkAllSampleReferences();
    }
    else
    {
        dynamic_cast<FrontendProcessor*>(mc)->setAllSampleReferencesCorrect();
    }
#endif
}

void AudioProcessorDriver::saveDeviceSettingsAsXml()
{
    
	ScopedPointer<XmlElement> deviceData = deviceManager != nullptr ?
                                           deviceManager->createStateXml():
                                           nullptr;

	if (deviceData != nullptr)
	{
		deviceData->writeToFile(getDeviceSettingsFile(), "");

		
	}
}

void GlobalSettingManager::setDiskMode(int mode)
{
	diskMode = mode;

	if (MainController* mc = dynamic_cast<MainController*>(this))
	{
		mc->getSampleManager().setDiskMode((MainController::SampleManager::DiskMode)mode);
	}
}

AudioDeviceDialog::~AudioDeviceDialog()
{

}


StandaloneProcessor::StandaloneProcessor()
{
	deviceManager = new AudioDeviceManager();
	callback = new AudioProcessorPlayer();

	wrappedProcessor = createProcessor();

    
	ScopedPointer<XmlElement> xml = AudioProcessorDriver::getSettings();

#if USE_BACKEND
	if(!CompileExporter::isExportingFromCommandLine()) 
		dynamic_cast<AudioProcessorDriver*>(wrappedProcessor.get())->initialiseAudioDriver(xml);
#else
	dynamic_cast<AudioProcessorDriver*>(wrappedProcessor.get())->initialiseAudioDriver(xml);
	dynamic_cast<FrontendProcessor*>(wrappedProcessor.get())->loadSamplesAfterSetup();

#endif

	
	
		
}


XmlElement * AudioProcessorDriver::getSettings()
{
	File savedDeviceData = getDeviceSettingsFile();

	return XmlDocument::parse(savedDeviceData);
}

void AudioProcessorDriver::initialiseAudioDriver(XmlElement *deviceData)
{
	if (deviceData != nullptr && deviceData->hasTagName("DEVICESETUP") && deviceManager->initialise(0, 2, deviceData, false) == String())
	{
		callback->setProcessor(dynamic_cast<AudioProcessor*>(this));

		deviceManager->addAudioCallback(callback);
		deviceManager->addMidiInputCallback(String(), callback);
	}
	else
	{
		deviceManager->initialiseWithDefaultDevices(0, 2);

		callback->setProcessor(dynamic_cast<AudioProcessor*>(this));

		deviceManager->addAudioCallback(callback);
		deviceManager->addMidiInputCallback(String(), callback);
	}
}


void GlobalSettingManager::setGlobalScaleFactor(double newScaleFactor)
{
	scaleFactor = newScaleFactor;
}

void AudioProcessorDriver::updateMidiToggleList(MainController* mc, ToggleButtonList* listToUpdate)
{
    
	ScopedPointer<XmlElement> midiSourceXml = dynamic_cast<AudioProcessorDriver*>(mc)->deviceManager->createStateXml();

	StringArray midiInputs = MidiInput::getDevices();

	if (midiSourceXml != nullptr)
	{
		for (int i = 0; i < midiSourceXml->getNumChildElements(); i++)
		{
			if (midiSourceXml->getChildElement(i)->hasTagName("MIDIINPUT"))
			{
				const String activeInputName = midiSourceXml->getChildElement(i)->getStringAttribute("name");

				const int activeInputIndex = midiInputs.indexOf(activeInputName);

				if (activeInputIndex != -1)
				{
					listToUpdate->setValue(activeInputIndex, true, dontSendNotification);
				}
			}
		}
	}
}


GlobalSettingManager::GlobalSettingManager()
{
	ScopedPointer<XmlElement> xml = AudioProcessorDriver::getSettings();

	if (xml != nullptr)
	{
		scaleFactor = (float)xml->getDoubleAttribute("SCALE_FACTOR", 1.0);
	}
}

void GlobalSettingManager::saveSettingsAsXml()
{
	ScopedPointer<XmlElement> settings = new XmlElement("GLOBAL_SETTINGS");

	settings->setAttribute("DISK_MODE", diskMode);
	settings->setAttribute("SCALE_FACTOR", scaleFactor);
	settings->setAttribute("MICRO_TUNING", microTuning);
	settings->setAttribute("TRANSPOSE", transposeValue);
	settings->setAttribute("SUSTAIN_CC", ccSustainValue);

#if USE_FRONTEND
	settings->setAttribute("SAMPLES_FOUND", allSamplesFound);
#endif

	settings->writeToFile(getGlobalSettingsFile(), "");

}

File GlobalSettingManager::getSettingDirectory()
{

#if JUCE_WINDOWS
#if USE_BACKEND
	String parentDirectory = File(PresetHandler::getDataFolder()).getFullPathName();
#else
	String parentDirectory = ProjectHandler::Frontend::getAppDataDirectory().getFullPathName();
#endif

	File parent(parentDirectory);

	if (!parent.isDirectory())
		parent.createDirectory();

#else

#if HISE_IOS
	String parentDirectory = File::getSpecialLocation(File::SpecialLocationType::userApplicationDataDirectory).getFullPathName();
#else

#if USE_BACKEND
	String parentDirectory = File(PresetHandler::getDataFolder()).getFullPathName();
#else
	String parentDirectory = ProjectHandler::Frontend::getAppDataDirectory().getFullPathName();
#endif
#endif

	File parent(parentDirectory);
#endif

	return parent;

}

void GlobalSettingManager::restoreGlobalSettings(MainController* mc)
{
	File savedDeviceData = getGlobalSettingsFile();

	ScopedPointer<XmlElement> globalSettings = XmlDocument::parse(savedDeviceData);

	

	GlobalSettingManager* gm = dynamic_cast<GlobalSettingManager*>(mc);

	gm->diskMode = globalSettings->getIntAttribute("DISK_MODE");
	gm->scaleFactor = globalSettings->getDoubleAttribute("SCALE_FACTOR", 1.0);
	gm->microTuning = globalSettings->getDoubleAttribute("MICRO_TUNING", 0.0);
	gm->transposeValue = globalSettings->getIntAttribute("TRANSPOSE", 0);
	gm->ccSustainValue = globalSettings->getIntAttribute("SUSTAIN_CC", 64);

	mc->setGlobalPitchFactor(gm->microTuning);

	mc->getEventHandler().setGlobalTransposeValue(gm->transposeValue);

	mc->getEventHandler().addCCRemap(gm->ccSustainValue, 64);
	mc->getSampleManager().setDiskMode((MainController::SampleManager::DiskMode)gm->diskMode);
}
