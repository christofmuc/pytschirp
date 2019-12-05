#pragma once

#include <pybind11/pybind11.h>

#include "SynthParameterDefinition.h"
#include "Patch.h"
#include "AutoDetection.h"

#include <algorithm>

namespace py = pybind11;

class WaitForEvent : public Thread {
public:
	WaitForEvent(std::function<bool()> hasHappened) : Thread("WaitForEvent"), hasHappened_(hasHappened) {
	}

	virtual void run() override {
		while (!threadShouldExit()) {
			if (hasHappened_()) {
				return;
			}
		}
	}

private:
	std::function<bool()> hasHappened_;
};

class MidiRequest {
public:
	typedef std::function<bool(MidiMessage const &)> TIsAnswerPredicate;
	MidiRequest(std::string const &midiOutput, MidiMessage const &request, TIsAnswerPredicate pred) : output_(midiOutput), request_(request), pred_(pred) {
	}

	MidiMessage blockForReply() {
		auto handler = midikraft::MidiController::makeOneHandle();
		bool answered = false;
		MidiMessage answer;
		midikraft::MidiController::instance()->addMessageHandler(handler, [this, &answered, &answer](MidiInput *source, MidiMessage const &message) {
			if (pred_(message)) {
				answer = message;
				answered = true;
			}
		});
		midikraft::MidiController::instance()->getMidiOutput(output_)->sendMessageNow(request_);
		// Busy wait thread
		WaitForEvent waiting([&answered]() { return answered; });
		waiting.startThread();
		if (waiting.waitForThreadToExit(2000)) {
			std::cout << "Got edit buffer from synth" << std::endl;
			midikraft::MidiController::instance()->removeMessageHandler(handler);
			return answer;
		}
		else {
			std::cerr << "Timeout while waiting for edit buffer midi message, failure" << std::endl;
			midikraft::MidiController::instance()->removeMessageHandler(handler);
			throw new std::runtime_error("PyTschirp: Timeout while waiting for edit buffer midi message");
		}
	}

private:
	std::string output_;
	MidiMessage request_;
	TIsAnswerPredicate pred_;
};

class PyTschirpInvalidAttribute {
public:
	void set(int newValue) {
		ignoreUnused(newValue);
		throw new std::runtime_error("PyTschirp: Can't set value of invalid attribute");
	}
};

template<typename PATCH, typename SYNTHPARAM, typename PARAM_ID>
class PyTschirpAttribute {
public:
	PyTschirpAttribute(std::shared_ptr<PATCH> patch, PARAM_ID const &param) : patch_(patch), def_(PATCH::find(param)) {
	}

	void set(int value) {
		// Depending on what type this object is, we might be successful or not

		if (py::int_(value).check()) {
			// This is an integer, use the integer setter
			int v = py::int_(value);
			setV(v);
		}
		else {
			throw std::runtime_error("PyTschirp: Type can not be set to parameter");
		}
	}

	void set(py::list newlist) {
		// Ups, this is a list. Only some synth parameters are lists (e.g. gated sequencer data), so we need some additional 
		// checks before we can set this...
		if (def_->sysexIndex() != def_->endSysexIndex()) {
			// This is indeed an array type of synth parameter
			if (py::len(newlist) != (def_->endSysexIndex() - def_->sysexIndex() + 1)) {
				throw std::runtime_error("PyTschirp: List length doesn't match parameter length");
			}
		}
		else {
			throw std::runtime_error("PyTschirp: Type can not be set to list");
		}
	}

	void setV(bool newValue) {
		patch_->setAt(def_->sysexIndex(), newValue ? 1 : 0);
	}

	void setV(int newValue) {
		patch_->setAt(def_->sysexIndex(), newValue);
	}

	int get() const {
		int value;
		if (def_->valueInPatch(*patch_, value)) {
			return value;
		}
		// Invalid
		throw new std::runtime_error("PyTschirp: Invalid attribute index in patch");
	}

	std::string asText() const {
		return def_->valueAsText(get());
	}

private:
	std::shared_ptr<PATCH> patch_;
	std::shared_ptr <midikraft::SynthParameterDefinition> def_;
};

template<typename PATCH, typename ATTRIBUTE, typename PARAM_ID>
class PyTschirp {
public:
	PyTschirp() {
		patch = std::make_shared<PATCH>();
	}

	PyTschirp(std::shared_ptr<midikraft::Patch> p) {
		// Downcast possible?
		auto correctPatch = std::dynamic_pointer_cast<PATCH>(p);
		if (!correctPatch) {
			throw std::runtime_error("PyTschirp: Can't downcast, wrong patch type! Program error");
		}
		patch = correctPatch;
	}

	ATTRIBUTE attr(PARAM_ID const &param) {
		return ATTRIBUTE(patch, param);
	}

	ATTRIBUTE get_attr(std::string const &attrName) {
		try {
			return ATTRIBUTE(patch, attrName);
		}
		catch (std::runtime_error &) {
			// That doesn't seem to exist... try with spaces instead of underscores in the name
			return ATTRIBUTE(patch, underscoreToSpace(attrName));
		}
	}

	void set_attr(std::string const &name, int value) {
		try {
			auto attr = ATTRIBUTE(patch, name);
			attr.setV(value);
		}
		catch (std::runtime_error &) {
			auto attr = ATTRIBUTE(patch, underscoreToSpace(name));
			attr.setV(value);
		}
	}

private:
	std::string underscoreToSpace(std::string const &input) {
		auto copy = input;
		std::replace(copy.begin(), copy.end(), '_', ' ');
		return copy;
	}

	std::shared_ptr<PATCH> patch;
};

template<typename SYNTH, typename TSCHIRP>
class PyTschirpSynth {
public:
	PyTschirpSynth() {
		synth_ = std::make_shared<SYNTH>();
	}

	void detect() {
		std::vector<std::shared_ptr<midikraft::SimpleDiscoverableDevice>> list;
		list.push_back(synth_);
		midikraft::AutoDetection autodetection;
		autodetection.autoconfigure(list);
	}

	bool detected() {
		return synth_->channel().isValid();
	}

	std::string location() const {
		auto result = String("MIDI IN: ") + synth_->midiInput() + String(", MIDI OUT: ") + synth_->midiOutput();
		return result.toStdString();
	}

	TSCHIRP editBuffer() {
		if (!detected()) {
			throw std::runtime_error("PyTschirp: Synth hasn't been detected yet - run detect() first and check if it worked");
		}

		// Let's see if this is possible
		auto editBufferCapability = std::dynamic_pointer_cast<midikraft::EditBufferCapability>(synth_);
		if (editBufferCapability) {
			// Block until we get the edit buffer back from the synth!
			auto request = editBufferCapability->requestEditBufferDump();
			MidiRequest requester(synth_->midiOutput(), request, [editBufferCapability](MidiMessage const &message) {
				return editBufferCapability->isEditBufferDump(message);
			});
			
			auto editBufferMessage = requester.blockForReply();

			auto patches = synth_->loadSysex({ editBufferMessage });
			if (patches.empty()) {
				throw std::runtime_error("PyTschirp: Failed to create edit buffer from reply, program error!");
			}
			if (!patches[0]) {
				throw std::runtime_error("PyTschirp: Failed to parse edit buffer, program error!");
			}
			return TSCHIRP(patches[0]);
		}
		else {
			std::cerr << "The " << synth_->getName() << " has no capability to recall the edit buffer, failed." << std::endl;
			throw std::runtime_error("PyTschirp: No edit buffer capability in device");
		}
	}

private:
	std::shared_ptr<SYNTH> synth_;
};
