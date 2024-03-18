import argparse
import subprocess
import os
from glob import glob

def status(message):
    print("\n>>>>>>>>>> " + message + "\n")

def sign(certificate):
    # Sign all plugins
    status("Signing plugins...")
    plugins = glob(os.path.join("tmp/VST3/ear-production-suite", "*.vst3"))
    plugins.append("tmp/VST3/ADM Export Source.vst3")
    for p in plugins:
        subprocess.run(["xcrun", "codesign", "--timestamp", "--options", "runtime", "-s", certificate, p])
        
    # Sign setup app bundle
    subprocess.run(["xcrun", "codesign", "--timestamp", "--options", "runtime",
                    "-s", certificate, "tmp/Setup EAR Production Suite.app"])
                    
    # Sign project upgrade gui app bundle
    status("Signing project upgrade gui app...")
    subprocess.run(["xcrun", "codesign", "--timestamp", "--options", "runtime",
                    "-s", certificate, "tmp/Tools/Project Upgrade Utility GUI.app"])

    # REAPER extension is naked dylib so no Info.plist to derive identifier from, specify on command line
    status("Signing reaper extension...")
    subprocess.run(["xcrun", "codesign", "--timestamp", "--options", "runtime",
                    "-i", "ch.ebu.eps.reaper_adm",
                    "-s", certificate, "tmp/UserPlugins/reaper_adm.dylib"])

    # Project upgrade command line exe has no Info.plist to derive identifier from, specify on command line
    status("Signing project upgrade cli tool...")
    subprocess.run(["xcrun", "codesign", "--timestamp", "--options", "runtime",
                    "-i", "ch.ebu.eps.reaper_project_upgrade",
                    "-s", certificate, "tmp/Tools/project_upgrade"])

def extract(fileName):
    status("Extracting...")
    subprocess.run(["tar", "-xvf", fileName])
    tarFile = fileName.replace("zip", "tar")
    subprocess.run(["rm", "-rf", "tmp/"])
    subprocess.run(["mkdir", "tmp/"])
    subprocess.run(["tar", "-xvf", tarFile, "-Ctmp/"])
    subprocess.run(["rm", tarFile])

def createDmg(outputname):
    status("Creating disk image...")
    dmgFile = outputname + ".dmg"
    volName = outputname
    subprocess.run(["hdiutil", "create", "-volname", volName, "-srcfolder", "./tmp", "-ov", "-format", "UDZO", dmgFile])
    
def signDmg(certificate, outputname):
    status("Signing disk image...")
    dmgFile = outputname + ".dmg"
    subprocess.run(["xcrun", "codesign", "-s", certificate, dmgFile])

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Sign EAR Production Suite distribution')
    parser.add_argument('certificate', metavar='certificate', type=str,
                        help='Name of the developer certificate with which to to sign code')
    parser.add_argument('artifact', metavar='artifact', type=str,
                        help='build artifact to be signed')
    parser.add_argument('outputname', metavar='outputname', type=str,
                        help='name of image to create (exc ext)')
    args = parser.parse_args()
    extract(args.artifact)
    sign(args.certificate)
    createDmg(args.outputname)
    signDmg(args.certificate, args.outputname)
