const locations = [
  {
    name: "Great Barrier Reef", 
    description: "Great Barrier Reef", 
    camera: {
      position: {
        x: 16567794.97, 
        y: -3402244.69,
        z:  1010477.31,
        spatialReference: { 
          wkid: 102100
        },
      },
      heading: 14.48,
      tilt: 35.54
    }
  },
  {
    name: "AusSeabed Marine Data", 
    description: "AusSeabed Marine Data", 
    camera: {
      position: {
        x: 15168078.420351503,
        y: -4967422.730831934,
        z: 9789897.624646775,
        spatialReference: { 
          wkid: 102100
        },
      },
      heading: 358.208687,
      tilt: 6.304827
    }
  },
  {
    name: "AusSeabed Marine Data", 
    description: "AusSeabed Marine Data", 
    camera: {
      position: {
        x: 15168078.420351503,
        y: -4967422.730831934,
        z: 9789897.624646775,
        spatialReference: { 
          wkid: 102100
        },
      },
      heading: 358.208687,
      tilt: 6.304827
    }
  }
];

/**
  @description Fly the camera without calling camera.viewer.flyto - we do this in here.
  @param {Cesium.viewer} viewer requires cesium viewer object
  @param {String} place requires a string to determine where to fly to
  @param {Number} duration requires a number to determine how long to should the flight takes.
*/
const flyCamera = (view, place, duration) => {
  if(view == null) {
    console.log("View object is not defined!")
    return;
  }

  locations.map((location) => {
    if(place === location.name) {
      // If the place is matched in the location array, we fly to camera.
      const options = {
        speedFactor: 0.2, // animation is 5 times slower than default
        easing: "out-quint" // easing function to slow down when reaching the target
      };
    
      view.goTo(location.camera, options);
      return true;
    }
  });
}

const defaultViewpoint = () => {
  return ({
    camera: {
      position: {
        x: 14798566.57611913,
        y: -3147250.482783899,
        z: 12059425.86571629,
        spatialReference: { 
          wkid: 102100
        },
      },
      heading: 3.78,
      tilt: 0.25
    }
  });
}

const moveCameraToAus = (view) => {
  const options = {
    speedFactor: 0.2, // animation is 5 times slower than default
    easing: "out-quint" // easing function to slow down when reaching the target
  };

  view.goTo({
    position: {
      x: 14798566.57611913,
      y: -3147250.482783899,
      z: 12059425.86571629,
      spatialReference: { 
        wkid: 102100
      },
    },
    heading: 3.78,
    tilt: 0.25
  }, options);
}