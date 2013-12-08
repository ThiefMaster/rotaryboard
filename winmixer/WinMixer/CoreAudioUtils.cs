using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using CoreAudio;

namespace WinMixer.CoreAudioUtils {
    static class Ext {
        public static IEnumerable<AudioSessionControl2> AsEnumerable(this SessionCollection sessions) {
            for (var i = 0; i < sessions.Count; i++) {
                yield return sessions[i];
            }
        }

        public static IEnumerable<MMDevice> AsEnumerable(this MMDeviceCollection devices) {
            for (var i = 0; i < devices.Count; i++) {
                yield return devices[i];
            }
        }
    }

    class VolumeControl {
        private Func<float> getVolume;
        private Action<float> setVolume;
        private Func<bool> isMuted;
        private Action<bool> setMuted;

        public VolumeControl(SimpleAudioVolume simpleVol) {
            getVolume = () => simpleVol.MasterVolume;
            setVolume = (volume) => simpleVol.MasterVolume = volume;
            isMuted = () => simpleVol.Mute;
            setMuted = (mute) => simpleVol.Mute = mute;
        }

        public VolumeControl(AudioEndpointVolume endpointVol) {
            getVolume = () => endpointVol.MasterVolumeLevelScalar;
            setVolume = (volume) => endpointVol.MasterVolumeLevelScalar = volume;
            isMuted = () => endpointVol.Mute;
            setMuted = (mute) => endpointVol.Mute = mute;
        }

        public int Volume {
            get {
                return (int)(Math.Round(getVolume() * 100));
            }
            set {
                setVolume(Math.Max(0, Math.Min(100, value)) / 100f);
            }
        }

        public bool Muted {
            get {
                return isMuted();
            }
            set {
                setMuted(value);
            }
        }
    }
}
