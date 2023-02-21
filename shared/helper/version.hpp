#pragma once

class Version {
public:
    Version() {};
    Version(int vMajor, int vMinor, int vRevision) {
        major = vMajor;
        minor = vMinor;
        revision = vRevision;
    };
    ~Version() {};

    bool operator>(const Version& other) const {
        if (major > other.major) {
            return true;
        }
        else if (major == other.major) {
            if (minor > other.minor) {
                return true;
            }
            else if (minor == other.minor) {
                if (revision > other.revision) {
                    return true;
                }
            }
        }
        return false;
    }

    bool operator<(const Version& other) const {
        return other > *this;
    }

    bool operator==(const Version& other) const {
        return major == other.major &&
            minor == other.minor &&
            revision == other.revision;
    }
    
    bool operator>=(const Version& other) const {
        return *this == other || *this > other;
    }
    
    bool operator<=(const Version& other) const {
        return *this == other || *this < other;
    }
	
	bool operator!=(const Version& other) const {
        return !(*this == other);
    }

    int major = 0;
    int minor = 0;
    int revision = 0;
};