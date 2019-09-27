<template>

	<v-sheet>
		{{ name }}
		<div class="d-flex flex-row">
			<v-slider
				v-for="n, key in values"
				:key="key"
				v-model="values[key]"
				vertical
				:min=0
				:max=63
				readonly
				track-color="transparent"
				track-fill-color="transparent"
			></v-slider>
		</div>
	</v-sheet>

</template>

<script>
	module.exports = {
		props: [
			'index'
		],
		computed: {
			values: {
				get: function () {
					return this.$root.data.data.config.project.sound.waveforms[this.index].values
				},
				set: function (newValues) {
					this.$root.data.data.config.project.sound.waveforms[this.index].values = newValues
				}
			},
			compare() {
				return 0
			},
			name() {
				return this.$root.data.data.config.project.sound.waveforms[this.index].name
			},
		}
	}
</script>

<style scoped>
.v-slider {
	text-align: center;
	width: 10px;
}
</style>