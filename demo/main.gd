extends Steam

func _ready():
	var init = init()
	var persona_name = get_persona_name()
	
	prints("init: " + str(init))
	prints("get_persona_name: " + persona_name)
