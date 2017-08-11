words = ['merc hyd', 'hi', 'merc pune', 'mercury']

function suggestion(id){
    str = document.getElementById(id).value
    if(str == null || str == undefined)
        return null
    result = []
    for(var i=0; i<words.length; i++){
        if(words[i].startsWith(str))
            result.push(words[i])
        
    }
    console.log(result)
}