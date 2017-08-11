function creditCard(cardNumber){
	result = {}
	result.valid = "false"
	result.number = null
	if(cardNumber == null || cardNumber == undefined){
		result.error="Card number invalid"
		return result

    }
    cardNumber = cardNumber.trim().split('-')
    if(cardNumber.length != 4){
        result.error="Improper card number format"
        return result
        
    }
    for(var i=0; i<4; i++){
        if(cardNumber[i].length != 4){
            result.error="Improper format"
            return result
        }
		else if(isNaN(Number(cardNumber[i]))){
			result.error = "Card number must only consist of numbers"
			return result
        }
    }
	result.valid = true
    result.number = cardNumber.join('-')
    return result
}