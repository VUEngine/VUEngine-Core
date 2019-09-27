<template>
	<v-form>

		<v-row>
			<v-col>

				<div
					v-for="n, key in $root.data.data.config.project.sound.waveforms"
					:key="key"
				>
					<s-waveform-preview
						:index=key
						class="float-left"
						@click="editKey = key"
					></s-waveform-preview>
				</div>

			</v-col>
		</v-row>
		<v-row>
			<v-col>

				<s-waveform-edit
					:index=editKey
				></s-waveform-edit>

			</v-col>
		</v-row>

	</v-form>
</template>

<script>
	module.exports = {
		data: () => ({
			editKey: 0
		})
	}
</script>

<style scoped>
</style>