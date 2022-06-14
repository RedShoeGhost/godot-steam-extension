extends Node

func _ready():
	var init = Steam.init()
	var persona_name = Steam.get_persona_name()
	
	prints("init: " + str(init))
	prints("get_persona_name: " + persona_name)
