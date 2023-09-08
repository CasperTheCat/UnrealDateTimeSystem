from datetime import date

parambase = """
#### Parameters

| Name | Type | Description |
| --- | --- | --- |
"""

returnbase = """
#### Returns
"""

paramprompt = """
### Instruction:
Summarise the parameter types for the following definition:

{}

Use 'void' if no parameters exist and describe the purpose of the parameter if it does.

### Response:
#### Parameters

| Name | Type | Description |
| --- | --- | --- |
"""

returnprompt = """
### Instruction:
Summarise the return types for the following definition:

{}

### Response:
#### Returns
"""

descprompt = """
### Instruction:
Given:

{}

{}

Create a brief explaination of the purpose of the function for the following:

{}

### Response:
"""

def GetName(x):
    try:
        codeLine = x[-1]
        codeParts = codeLine.split(" ")
        if codeLine.startswith("virtual"):
            return codeParts[2].split("(")[0]
        else:
            return codeParts[1].split("(")[0]
    except:
        print(x)
        return "BADNAME"

bannedStarts = [
    "UCLASS",
    "class",
]

bannedEnds = [
    ""
]

def Eat(lines):
    packetOpen = False
    commentOpen = False
    codeLinePacked = False
    codeLineExpected = False
    packet = []
    for l in range(len(lines)):
        cline = lines[l].lstrip().rstrip()

        if cline.rstrip().lstrip() in bannedEnds and packetOpen:
            if commentOpen:
                pass
            if codeLinePacked:
                packetOpen = False
                return lines[l:], packet           
            if codeLineExpected:
                return lines[l:], None

        if cline.startswith("//"):
            packetOpen = True
            codeLineExpected = False

        if cline.startswith("/*"):
            packetOpen = True
            commentOpen = True

        if codeLineExpected:
            for ban in bannedStarts:
                if cline.startswith(ban):
                    return lines[l:], None

        if packetOpen:
            packet.append(cline)

            if codeLineExpected:
                codeLinePacked = True
                codeLineExpected = False

        if cline.startswith("//"):
            codeLineExpected = True

        if cline.startswith("*/") and commentOpen:
            commentOpen = False
            codeLineExpected = True



    return [], []

def Parse(lines):
    baoguo = []

    while len(lines) != 0:
        lines, packet = Eat(lines)
        if packet is not None:
            baoguo.append(packet)

    return baoguo

if __name__ == "__main__":
    import os
    import sys
    import datetime
    from dateutil import parser
    from llama_cpp import Llama
    LLM = Llama(model_path="Z:\StableDiffusion\Models\LLama213\llama-2-70b-chat.ggmlv3.q4_K_S.bin", n_gqa=8, n_ctx=2048, n_gpu_layers=43)
    #LLM = Llama(model_path="Z:\StableDiffusion\Models\LLama213\llama-2-13b.ggmlv3.q6_K.bin", n_ctx=2048, n_gpu_layers=83)

    fileTypes = ["h"]
    #MAKE_ARGS="-DLLAMA_CUBLAS=on" FORCE_CMAKE=1 pip install llama-cpp-python
    # https://pypi.org/project/llama-cpp-python/

    for r,d,f in os.walk("Source"):
        # For Files
        for i in f:
            iSplit = i.split(".")
            if iSplit[-1] in fileTypes:
                with open(os.path.join(r,i), "r") as fd:
                    lines = fd.read().split("\n")
                    Parses = Parse(lines)
                    for mm in Parses:
                        if len(mm) == 0:
                            continue 
                        retprompt = returnprompt.format(
                            "\n".join(mm)
                        )

                        parprompt = paramprompt.format(
                            "\n".join(mm)
                        )

                        #print(promptl)
                        nnName = GetName(mm)

                        paramoutput = LLM(parprompt, max_tokens=128, stop=["### Instruction:"])
                        retoutput = LLM(retprompt, max_tokens=128, stop=["### Instruction:"])


                        prompt = descprompt.format(
                            parambase + paramoutput["choices"][0]["text"],
                            returnbase + retoutput["choices"][0]["text"],
                            "\n".join(mm)
                        )
                        print(prompt, file=sys.stderr)

                        output = LLM(prompt, max_tokens=256, stop=["### Instruction:"])

                        print("###", nnName)
                        print(output["choices"][0]["text"])
                        print(parambase + paramoutput["choices"][0]["text"])
                        print(returnbase + retoutput["choices"][0]["text"])

                        print(output["choices"][0]["text"], file=sys.stderr)

                        print("\n\n---\n\n\n\n\n")
                        

            # # create a text prompt
            # prompt = "Q: What are the names of the days of the week? A:"

            # # generate a response (takes several seconds)
            # output = LLM(prompt)

            # # display the response
            # print(output["choices"][0]["text"])

