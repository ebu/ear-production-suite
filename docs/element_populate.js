window.addEventListener('load', function () {
	for(let el of document.querySelectorAll('[data-type]')){
		if(!el.hasAttribute("data-source")){
			console.error("Element with data-type has no data-source:", el)
		}
	}
	
	for(let el of document.querySelectorAll('[data-source]')){
		if(!el.hasAttribute("data-type")){
			console.error("Element with data-source has no data-type:", el)
		}
	}
	
	fetch("version_info.json")
	.then(response => response.json())
	.then(json => {
		console.log("data-sources imported;", json)

		for(let el of document.querySelectorAll('[data-source]')){
			if(el.hasAttribute("data-type")){
				let s = el.getAttribute("data-source");
				let t = el.getAttribute("data-type");
				if(!json.hasOwnProperty(s)){
					console.error("Elements data-source does not exist:", el)
				} else if(typeof(json[s]) != "string") {
					// Force only strings to keep it simple for EPS parser
					console.error("data-source is not a string:", s, json[s])
				} else if(t == "href") {
					el.href = json[s]	
				} else if(t == "innertext") {
					el.innerText = json[s]	
				} else {
					console.error("Elements data-type is not supported:", el)
				}
			}
		}

	});
})
