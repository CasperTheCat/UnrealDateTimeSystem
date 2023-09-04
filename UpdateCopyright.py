from datetime import date


copyrightNotice = "Copyright Acinonyx Ltd. {}. All Rights Reserved."

def GenerateCopyrightInformation(year):
    # Generate Copyright with the correct tags
    outString = ["// " + copyrightNotice.format(year)]
    return outString

def ParseCopyright(x, notice):
    lines = x.read().split("\n")

    # Bail on UHT files
    for i in lines[:10]:
        if "Generated code exported from UnrealHeaderTool" in i:
            return False, None

    # Is the line a comment?
    if not lines[0].startswith("//"):
        # No Copyright Info?
        # Warn and add
        lines = notice + lines
    else:
        # Check the case where Epic Games has a copyright notice
        # That notice needs to be preserved alongside our notice
        if "Copyright Epic Games" in lines[0]:
            lines = notice + lines

        # We need to remove the dummy template if it's here
        elif "Fill out your copyright notice in the Description page of Project Settings" in lines[0] or "[TEMPLATE_COPYRIGHT]" in lines[0]:
            lines = notice + lines[1:]
            
        # Is there a generated notice here?
        # If there is, we can just update the inner portions
        elif lines[0].startswith("// StartCopyrightNotice"):
            lines[1] = notice[0]

        elif lines[0].startswith("// Copyright"):
            lines[0] = notice[0]

    return True, "\n".join(lines)


if __name__ == "__main__":
    import os
    import sys
    import datetime
    from dateutil import parser

    cYear = datetime.datetime.now().year
    notice = GenerateCopyrightInformation(cYear)
    bUseCommitDate = False
    bCYearIsFirstPublishDate = False

    if len(sys.argv) > 1 and sys.argv[1] == "commit":
        print("Fixing ©")
        bUseCommitDate = True

        if len(sys.argv) > 2:
            cYear = int(sys.argv[2])
            bCYearIsFirstPublishDate = True

    fileTypes = ["h", "cpp", "cs"]

    test = 0
    # Update in Source Dir
    for r,d,f in os.walk("Source"):
        # For Files
        print(f)
        for i in f:
            iSplit = i.split(".")
            if iSplit[-1] in fileTypes:
                fToOpen = os.path.join(r,i)
                with open(fToOpen, "r+") as handle:
                    if bUseCommitDate and bCYearIsFirstPublishDate:
                        yearString = os.popen("git log -1 --format=%cd {}".format(fToOpen)).read()
                        dtString = parser.parse(yearString).year

                        print(dtString, cYear)

                        if dtString > cYear:
                            lNotice = GenerateCopyrightInformation(dtString)
                        else:
                            lNotice = GenerateCopyrightInformation(cYear)
                        
                    else:
                        lNotice = notice
                        
                    bSuccess, modifiedText = ParseCopyright(handle, lNotice)

                    if bSuccess:
                        print("Updating file: {}".format(i))
                        handle.seek(0)
                        handle.write(modifiedText)
                        handle.close()


    # # Update in Plugins
    # for r,d,f in os.walk("./Plugins/AnimTools/Source"):
    #     # For Files
    #     for i in f:
    #         iSplit = i.split(".")
    #         if iSplit[-1] == "h" and not iSplit[-2] == "generated":
    #             with open(os.path.join(r,i), "r") as f:
    #                 ParseCopyright(i, f)

