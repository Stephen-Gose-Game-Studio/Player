/*
 * This file is part of EasyRPG Player.
 *
 * EasyRPG Player is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EasyRPG Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
 */

// Headers
#include "audio_midi.h"

#include <memory>
#include "audio_resampler.h"
#include "output.h"
#include "system.h"

#include "decoder_fluidsynth.h"
#include "decoder_fmmidi.h"
#include "decoder_midigeneric.h"
#include "decoder_wildmidi.h"

void MidiDecoder::GetFormat(int &freq, AudioDecoder::Format &format, int &channels) const {
	freq = frequency;
	format = AudioDecoder::Format::S16;
	channels = 2;
}

bool MidiDecoder::SetFormat(int frequency, AudioDecoder::Format format, int channels) {
	if (frequency != EP_MIDI_FREQ || channels != 2 || format != AudioDecoder::Format::S16)
		return false;

	return true;
}


static struct {
	bool fluidsynth = true;
	bool wildmidi = true;
} works;

std::unique_ptr<AudioDecoder> MidiDecoder::Create(Filesystem_Stream::InputStream &stream, bool resample) {
	std::unique_ptr<AudioDecoder> mididec;
	std::string error_message;

#if defined(HAVE_FLUIDSYNTH) || defined(HAVE_FLUIDLITE)
	if (works.fluidsynth && FluidSynthDecoder::Initialize(error_message)) {
		mididec = std::make_unique<GenericMidiDecoder>(new FluidSynthDecoder());
	} else if (!mididec && works.fluidsynth) {
		Output::Debug("{}", error_message);
		works.fluidsynth = false;
	}
#endif
#ifdef HAVE_WILDMIDI
	if (!mididec && works.wildmidi && WildMidiDecoder::Initialize(error_message)) {
		mididec = std::make_unique<GenericMidiDecoder>(new WildMidiDecoder());
	} else if (!mididec && works.wildmidi) {
		Output::Debug("{}", error_message);
		works.wildmidi = false;
	}
#endif
#if WANT_FMMIDI == 1
	if (!mididec) {
		mididec = std::make_unique<GenericMidiDecoder>(new FmMidiDecoder());
	}
#endif

#ifdef USE_AUDIO_RESAMPLER
	if (mididec && resample) {
		mididec = std::make_unique<AudioResampler>(std::move(mididec));
	}
#endif

	return mididec;
}
