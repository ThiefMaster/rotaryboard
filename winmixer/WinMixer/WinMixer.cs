using CoreAudio;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;
using WinMixer.CoreAudioUtils;

namespace WinMixer {
    public static class Devices {
        private static MMDeviceEnumerator devEnum = new MMDeviceEnumerator();

        private static IEnumerable<MMDevice> EnumDevices(EDataFlow dataFlow) {
            return devEnum.EnumerateAudioEndPoints(dataFlow, DEVICE_STATE.DEVICE_STATE_ACTIVE).AsEnumerable();
        }

        public static IEnumerable<MMDevice> EnumPlaybackDevices() {
            return EnumDevices(EDataFlow.eRender);
        }

        public static IEnumerable<MMDevice> EnumCaptureDevices() {
            return EnumDevices(EDataFlow.eCapture);
        }


        public static MMDevice DefaultPlayback {
            get {
                return devEnum.GetDefaultAudioEndpoint(EDataFlow.eRender, ERole.eMultimedia);
            }
        }

        public static MMDevice DefaultCapture {
            get {
                return devEnum.GetDefaultAudioEndpoint(EDataFlow.eCapture, ERole.eMultimedia);
            }
        }
    }

    public class Mixer {
        public static string DEVICE = Guid.NewGuid().ToString();
        public static string SYSTEM = Guid.NewGuid().ToString();

        private MMDeviceEnumerator devEnum = new MMDeviceEnumerator();
        private MMDevice dev = null;

        public Mixer() {
            dev = Devices.DefaultPlayback;
        }

        public Mixer(MMDevice device) {
            dev = device;
        }

        public IEnumerable<string> EnumPrograms() {
            return from session in dev.AudioSessionManager2.Sessions.AsEnumerable()
                   let pid = (int)session.GetProcessID
                   where pid != 0
                   join process in Process.GetProcesses() on pid equals process.Id
                   select process.ProcessName;
        }

        // Retrieves a volume controller for a given ProcessName's session.
        // If no name is provided, the system sound session is returned.
        private VolumeControl GetSessionControl(string name = null) {
            foreach (var sessionControl in dev.AudioSessionManager2.Sessions.AsEnumerable()) {
                int pid = (int)sessionControl.GetProcessID;
                if (name == null && pid == 0) {
                    return new VolumeControl(sessionControl.SimpleAudioVolume);
                }
                else if (name != null && pid != 0) {
                    string processName;
                    try {
                        processName = Process.GetProcessById(pid).ProcessName;
                    }
                    catch (ArgumentException) {
                        // Process doesn't exist anymore
                        continue;
                    }
                    if (string.Equals(processName, name, StringComparison.OrdinalIgnoreCase)) {
                        return new VolumeControl(sessionControl.SimpleAudioVolume);
                    }
                }
            }
            return null;
        }

        private VolumeControl GetGlobalControl() {
            return new VolumeControl(dev.AudioEndpointVolume);
        }

        private VolumeControl GetVolumeControl(string name) {
            if (name == null) {
                throw new ArgumentNullException();
            }
            else if (name == DEVICE) {
                return GetGlobalControl();
            }
            else if (name == SYSTEM) {
                return GetSessionControl();
            }
            else {
                return GetSessionControl(name);
            }
        }

        #region Muting
        public bool? IsMuted(string name) {
            var volumeControl = GetVolumeControl(name);
            if (volumeControl == null) {
                return null;
            }
            return volumeControl.Muted;
        }

        public bool? SetMuted(string name, bool mute) {
            var volumeControl = GetVolumeControl(name);
            if (volumeControl == null) {
                return null;
            }
            volumeControl.Muted = mute;
            return volumeControl.Muted;
        }

        public bool? ToggleMuted(string name) {
            var volumeControl = GetVolumeControl(name);
            if (volumeControl == null) {
                return null;
            }
            volumeControl.Muted = !volumeControl.Muted;
            return volumeControl.Muted;
        }
        #endregion

        #region Volume
        public int? GetVolume(string name) {
            var volumeControl = GetVolumeControl(name);
            if (volumeControl == null) {
                return null;
            }
            return volumeControl.Volume;
        }

        public int? SetVolume(string name, int volume) {
            var volumeControl = GetVolumeControl(name);
            if (volumeControl == null) {
                return null;
            }
            volumeControl.Volume = volume;
            return volumeControl.Volume;
        }

        public int? ChangeVolume(string name, int delta) {
            var volumeControl = GetVolumeControl(name);
            if (volumeControl == null) {
                return null;
            }
            volumeControl.Volume += delta;
            return volumeControl.Volume;
        }
        #endregion
    }
}
