
function load() {

	var colors = {
		white: 0xffffffff,
		peru: 0xCD853Fff,
		copper: 0xda8a67ff,
		steel: 0xB0C4DEff,
	}

	var metallic = 16

	var models = {
		droplet: "models/fluid.stl",
	}

	sim.item.new({
		name: "log",
		fuel: { type: "chemical", energy: 10*1000000 },
		parts: [{
			hdSTL: "models/wood.stl",
			paint: colors.peru,
			scale: [0.5,0.5,0.5],
		}],
	})

	sim.item.new({
		name: "iron-ore",
		armV: 0.2,
		minable: true,
		parts: [{
			hdSTL: "models/iron-ore.stl",
			paint: 0xb25607ff,
			gloss: metallic,
			scale: [0.6,0.6,0.6],
			translate: [-0.05,0,0],
		}],
	})

	sim.item.new({
		name: "copper-ore",
		armV: 0.2,
		minable: true,
		parts: [{
			hdSTL: "models/copper-ore.stl",
			paint: 0xAFEEEEff,
			gloss: metallic,
			scale: [0.6,0.6,0.6],
		}],
	})

	sim.item.new({
		name: "coal",
		armV: 0.2,
		minable: true,
		fuel: { type: "chemical", energy: 20*1000000 },
		parts: [{
			hdSTL: "models/coal.stl",
			paint: 0x444444ff,
			gloss: metallic,
			scale: [0.6,0.6,0.6],
			translate: [0.05,0,0],
		}],
	})

	sim.item.new({
		name: "stone",
		armV: 0.2,
		minable: true,
		parts: [{
			hdSTL: "models/stone.stl",
			paint: 0x999999ff,
			gloss: metallic,
			scale: [0.6,0.6,0.6],
		}],
	})

	sim.item.new({
		name: "copper-ingot",
		parts: [{
			hdSTL: "models/ingot.stl",
			paint: colors.copper,
			gloss: metallic,
			scale: [0.95,0.95,0.95],
			translate: [0,-0.01,0],
		}],
	})

	sim.item.new({
		name: "copper-sheet",
		armV: -0.1,
		parts: [{
			hdSTL: "models/sheet-hd.stl",
			ldSTL: "models/sheet-ld.stl",
			paint: colors.copper,
			gloss: metallic,
		}],
	})

	sim.item.new({
		name: "steel-ingot",
		parts: [{
			hdSTL: "models/ingot.stl",
			paint: colors.steel,
			gloss: metallic,
			scale: [0.95,0.95,0.95],
			translate: [0,-0.01,0],
		}],
	})

	sim.item.new({
		name: "steel-sheet",
		armV: -0.1,
		parts: [{
			hdSTL: "models/sheet-hd.stl",
			ldSTL: "models/sheet-ld.stl",
			paint: colors.steel,
			gloss: metallic,
		}],
	})

	sim.item.new({
		name: "brick",
		armV: 0.1,
		parts: [{
			hdSTL: "models/brick-hd.stl",
			ldSTL: "models/brick-ld.stl",
			paint: 0x888888ff,
		}],
	})

	sim.item.new({
		name: "copper-wire",
		parts: [{
			hdSTL: "models/copper-wire-roll-hd.stl",
			ldSTL: "models/copper-wire-roll-ld.stl",
			paint: 0xda8a67ff,
			translate: [0,0.25,0],
		},{
			hdSTL: "models/copper-wire-end-hd.stl",
			ldSTL: "models/copper-wire-end-ld.stl",
			paint: 0x444444ff,
			translate: [0,0.5,0],
		},{
			hdSTL: "models/copper-wire-end-hd.stl",
			ldSTL: "models/copper-wire-end-ld.stl",
			paint: 0x444444ff,
			translate: [0,0,0],
		}],
	})

	sim.item.new({
		name: "circuit-board",
		armV: 0.45,
		parts: [{
			hdSTL: "models/circuit-board.stl",
			paint: 0x228800ff,
		}],
	})

	sim.item.new({
		name: "pipe",
		armV: -0.18,
		parts: [{
			hdSTL: "models/pipe-item-hd.stl",
			ldSTL: "models/pipe-item-ld.stl",
			paint: 0xffa500ff,
			gloss: metallic,
			scale: [0.7,0.7,0.7],
			translate: [0,0.31,0],
		}],
	})
}

function update() {
}