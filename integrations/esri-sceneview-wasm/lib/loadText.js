// This file is providing function to load content of each clicked points.

// Define all information here
const contents = [
  // Place holder / null
  {
    name: "Null",
    title: "{placeholder}",
    imgHTML: "{placeholder}",
    modelInfo: "{placeholder}",
    attribution: "{placeholder}",
    textAttr: "{placeholder}",
    cont: "{placeholder}"
  },
  // Great Barrier Reef
  {
    name: "Great Barrier Reef",
    title: "Great Barrier Reef, sea floor data",
    imgHTML: "<img src=\"./img/models/bathymetry.jpg\" alt=\"Great Barrier Reef's Bathymetry\" />",
    modelInfo: "3D bathymetry of Great Barrier Reef containing more than 360 million points with a resolution of 30m.",
    attribution: "Model courtesy of Geoscience Australia",
    // rel="noopener noreferrer" prevent tabnabbing, visit https://www.freecodecamp.org/news/how-to-use-html-to-open-link-in-new-tab for more detail.
    textAttr: "This article uses material from the Geoscience Australia article <a href=\"https://www.ga.gov.au/about/projects/marine/northern-australia-bathymetry\" target=\"_blank\" rel=\"noopener noreferrer\">\"Great Barrier Reef sea floor data\"</a> published by the Australian Government, which is released under the <a href=\"https://creativecommons.org/licenses/by-sa/4.0/legalcode\" target=\"_blank\" rel=\"noopener noreferrer\">Creative Commons Attribution-Share-Alike License 4.0 International</a>.",
    cont: "Combining historical and new sea floor mapping data, known as bathymetry, the view of the shape of the reef has been improved from 250m resolution to 30m resolution.<br/><br/>The data covers an area of 1.5 million square kilometres and will help us better understand the sea floor structure of the Great Barrier Reef. This will be used for preservation projects, tidal current and wave modelling and a range of other applications.<br/><br/>There are currently some 3000 identified coral reefs in the Great Barrier Reef Marine Park but we hope this data will help to discover even more, which will support management and conservation of the marine park.<br/><br/>This data is the first of a range of datasets that will provide 30m bathymetry grids around large parts of the Australian coastline.<br/><br/>Geoscience Australia, <a href=\"https://www.jcu.edu.au/\" target=\"_blank\" rel=\"noopener noreferrer\">James Cook University</a> and the <a href=\"http://www.hydro.gov.au/\" target=\"_blank\" rel=\"noopener noreferrer\">Australian Hydrographic Service</a> have worked together as part of a four-year project to provide legal certainty for Australia’s maritime boundaries. This project collated millions of dollars’ worth of existing datasets from government sources including the Australian Hydrographic Service, along with new data.<br/><br/>This work also demonstrates what we can achieve by combining data and making it openly available.<br/><br/>This sea floor data can be used for policy, planning and scientific work. For example, this data is an important input for oceanographic modelling, which we can use to enhance our knowledge of climate change impacts, marine biodiversity, and species distribution.<br/><br/>It will also improve modelling of tides and ocean currents, including helping to better predict storm surges on the coast."
  },
  // Great Barrier Reef sea floor data
  {
    name: "AusSeabed Marine Data",
    title: "AusSeabed Marine Data, 50m Multibeam Dataset of Australia 2018",
    imgHTML: "<img src=\"./img/models/ausseabed_50m_multi.jpg\" alt=\"50m Multibeam Dataset of Australia 2018 from AusSeabed Marine Data\" />",
    modelInfo: "Multibeam dataset of Australia containing more than 1.86 billion unique points with a resolution of 50m.",
    attribution: "Model courtesy of Geoscience Australia",
    textAttr: "This article uses material from the Geoscience Australia article <a href=\"https://ecat.ga.gov.au/geonetwork/srv/eng/catalog.search#/metadata/124063\" target=\"_blank\" rel=\"noopener noreferrer\">\"50m Multibeam Dataset of Australia 2018\"</a> published by the Australian Government, which is released under the <a href=\"https://creativecommons.org/licenses/by-sa/4.0/legalcode\" target=\"_blank\" rel=\"noopener noreferrer\">Creative Commons Attribution-Share-Alike License 4.0 International</a>.",
    cont: "The AusSeaBed Portal provides access to publically available acoustic datasets such as bathymetry, backscatter, side scan sonar data and other marine-related products, as well as a suite of analytical assessment tools to maximise the value of the data. This interface allows users to explore seafloor mapping products across Australia’s marine jurisdiction prior to downloading. For more information about the program, please visit the <a href=\"http://www.ausseabed.gov.au/\" target=\"_blank\" rel=\"noopener noreferrer\">AusSeabed website</a>.<br/><br/>This dataset contains all multibeam bathymetry data held by Geoscience Australia as of June 2018. It was compiled from data holdings lying within the offshore area of Australia, including international waters, and gridded at 50m. Source information can be found in the associated vector product, <a href=\"http://pid.geoscience.gov.au/dataset/ga/116321\" target=\"_blank\" rel=\"noopener noreferrer\">AusSeabed Bathymetry Holdings</a>. This dataset is accessible via clip, zip & ship enabled web mapping service on Geoscience Australia’s <a href=\"http://marine.ga.gov.au\" target=\"_blank\" rel=\"noopener noreferrer\">AusSeabed website</a>. In future, this product will be delivered dynamically through the AusSeabed portal (in development)."
  },
  // 3rd dataset
  {
    name: "Null",
    title: "{placeholder}",
    imgHTML: "{placeholder}",
    modelInfo: "{placeholder}",
    attribution: "{placeholder}",
    textAttr: "{placeholder}",
    cont: "{placeholder}"
  }
  // add more here...
];

// Return the whole array of content;
const allContent = () => {
  if (!contents) {
    console.log("Content Undefined!");
    return;
  }
  return contents;
}

// Return specfic content object
const selectContent = (place) => {
  // console.log("place: "+place);
  for (var i = 0; i < contents.length; i++) {
    if (contents[i].name === place) {
      return contents[i];
    }
  }
  console.log("No content is found or loaded!.");
  return null;
}

// Calling own function and specific return attribute
const selectAttribute = (place) => {
  let content = selectContent(place);
  if (!content)
    return null;
  return content.attribution;
}

// Return all information 
const updateInnerText = (place) => {
  let content = selectContent(place);
  document.getElementById("descTitle").innerHTML = content.title;
  document.getElementById("introImg").innerHTML = content.imgHTML;
  document.getElementById("descParagraph").innerHTML = content.cont;
  document.getElementById("descModelInfo").innerHTML = content.modelInfo;
  document.getElementById("descAttr").innerHTML = content.attribution;
  document.getElementById("descTextAttr").innerHTML = content.textAttr;
}