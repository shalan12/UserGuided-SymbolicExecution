var utils = {
	function isEmptyObject(obj) 
	{
	  return !Object.keys(obj).length;
	}
	function getRandomInt()
	{
	  var randomNumber;
	  var found;
	 do 
	 {
		found = true;
		randomNumber = Math.random();
		randomNumber = randomNumber.toString().substring(2,randomNumber.length);

		for (var key in map) 
		{
		  if (map.hasOwnProperty(key) &&  randomNumber == key) 
		  {
			found = false
			break
		  }
		}

	  }while (!found);
	  return randomNumber;
	}
};