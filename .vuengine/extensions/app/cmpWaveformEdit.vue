<template>

	<div>
		<s-input
			type="text"
			:data=$root.data.data.config.project.sound.waveforms[index].name
			direct-data="true"
			label="Name"
		></s-input>
		<div :class="['d-flex', 'flex-row', {changed: !compare}]">
			<div
				v-for="n, key in values"
				:key="key"
				class="slider-wrapper"
			>
				{{ key + 1 }}
				<v-slider
					v-model="values[key]"
					vertical
					:min=0
					:max=63
				></v-slider>
				<b>{{values[key]}}</b>
				<s-input
					type="number"
					:data=values[key]
					direct-data="true"
					:min=0
					:max=63
				></s-input>
			</div>
		</div>
	</div>

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
				var data = this.$root.data
				var key = this.index
				return data.data.config.project.sound.waveforms[key].values.every(function(value, index) {
					return value === data.lastSavedData.config.project.sound.waveforms[key].values[index]
				})
			}
		}

	}
</script>

<style scoped>
.slider-wrapper {
	text-align: center;
	width: 3.125%;
}
</style>