import subprocess

images = ["aab(1).ppm"]
messages = ["A", "B"]

for x in images:
	for y in messages:
		print("Encoding "+y+" In "+x)
		subprocess.call("./steg \""+x+"\" -e -m \""+y+"\" -o \"ValidatorOut/"+x+y+"\"", shell=True)
		subprocess.call("./steg \""+x+"\" -d \"ValidatorOut/"+x+y+"\"", shell=True)