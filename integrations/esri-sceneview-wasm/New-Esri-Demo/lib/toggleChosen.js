// control css whether button is selected.
const onToggleChosen = (element) => {
  let ele = document.getElementById(element.target.id ? element.target.id : element.target.alt);
  places.map((place) => {
    // disable others clicked css effect.
    var chosen = document.getElementsByClassName("btn "+place.css_class+" chosen");
    // if we found something in this collection
    if(chosen.length > 0) {
      // Hack: Remove chosen from first one in collection;
      // We already have one loop above, don't want to nest loop.
      chosen[0].classList.toggle("chosen");
      return true;
    }
  })
  ele.classList.toggle("chosen");
}

// Force these element to be chosen
const toggleDefaultChosen = (name) => {
  document.getElementById(name).classList.toggle("chosen");
  document.getElementById("label "+name).classList.toggle("chosen");
}

// Cancel all chosen effect
const cancelAllChosen = () => {
  let chosenBtn = document.getElementsByClassName("btn chosen");
  if(chosenBtn.length > 0) {
    chosenBtn[0].classList.toggle("chosen");
  }
  
  let chosenLabel = document.getElementsByClassName("label chosen");
  if(chosenLabel.length > 0) {
    chosenLabel[0].classList.toggle("chosen");
  }
}